#pragma once

#include <print>
#include <format>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <boost/thread/synchronized_value.hpp>

#include "./json.hpp"

using nlohmann::json;

namespace detail{
class Resource {
public:
    virtual ~Resource() = default;

    virtual json serialize() const = 0;
    virtual void deserialize(const json& j) = 0;

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

protected:
    Resource() = default;
};
} // namespace detail

using Resource = boost::synchronized_value<std::unique_ptr<detail::Resource>>;

class ResourceManager {
private:
    ResourceManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
    std::mutex mtx;

    std::shared_ptr<Resource> newResource(std::unique_ptr<detail::Resource> &&r) {
        return std::make_shared<Resource>(boost::synchronized_value<std::unique_ptr<detail::Resource>>(std::move(r)));
    }

    std::shared_ptr<Resource> newResource(detail::Resource *r) {
        return std::make_shared<Resource>(boost::synchronized_value<std::unique_ptr<detail::Resource>>(std::unique_ptr<detail::Resource>(r)));
    }

public:
    static ResourceManager& instance() {
        static ResourceManager instance;
        return instance;
    }

    std::unordered_set<std::string> getResourceIds() {
        std::lock_guard<std::mutex> lock(mtx);
        std::unordered_set<std::string> result;
        for (const auto& [id, _] : resources) {
            result.insert(id);
        }
        return result;
    }

    void create(const std::string_view _id, detail::Resource *resource) {
        std::println(stderr, "Creating resource: {}", _id);
        std::lock_guard lock(mtx);
        std::string id(_id);
        resources.insert_or_assign(id, newResource(resource));
    }

    void create(const std::string_view _id, std::unique_ptr<detail::Resource> &&resource) {
        std::println(stderr, "Creating resource: {}", _id);
        std::lock_guard lock(mtx);
        std::string id(_id);
        resources.insert_or_assign(id, newResource(std::move(resource)));
    }

    void destroy(const std::string_view _id) {
        std::println(stderr, "Destroying resource: {}", _id);
        std::lock_guard lock(mtx);
        std::string id(_id);
        auto res = resources.erase(id);
        if (res == 0) {
            std::println(stderr, "WARN: unable to delete resource {}", id);
        }
    }

    std::weak_ptr<Resource> getResource(const std::string_view _id) {
        std::string id(_id);
        std::lock_guard lock(mtx);
        auto it = resources.find(id);
        if (it == resources.end()) {
            // Resource either doesn't exist or was removed
            return std::weak_ptr<Resource>{}; // empty ptr
        }

        // We return the pointer to the resource.
        // The caller will then call synchronize() on this returned object.
        return it->second;
    }

    json serialize() {
        std::lock_guard lock(mtx);
        json result = json::object();

        for (const auto& [id, res] : resources) {
            auto locked = res->synchronize();
            auto serialized = (*locked)->serialize();
            result[id] = serialized;
        }

        return json{{"resources", result}};
    }

    void deserialize(const json& j) {
        if (!j.contains("resources")) return;

        const auto& res_json = j.at("resources");
        std::lock_guard lock(mtx);

        for (const auto& [id, data] : res_json.items()) {
            auto it = resources.find(id);
            if (it == resources.end()) {
                std::println(stderr, "Resource with ID {} not found in registry, Skipping", id);
                continue;
            }

            auto locked = it->second->synchronize();
            (*locked)->deserialize(data);
        }
    }
};
