#pragma once

#include <algorithm>
#include <iostream>
#include <print>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <boost/thread/synchronized_value.hpp>

#include "./json.hpp"

using nlohmann::json;

class _Resource {
public:
    virtual ~_Resource() = default;

    virtual json serialize() const = 0;
    virtual void deserialize(const json& j) = 0;
    virtual std::string_view getId() const = 0;

    _Resource(const _Resource&) = delete;
    _Resource& operator=(const _Resource&) = delete;

protected:
    _Resource() = default;
};

using Resource = boost::synchronized_value<_Resource*>;
class ResourceManager {
private:

    // struct SynchronizedResource {
    //     std::unique_ptr<_Resource>> resource;
    //     std::unique_ptr<boost::synchronized_value<_Resource*>> synchronized;
    // }

    ResourceManager() = default;

    // Container of uniquely owned resources
    std::unordered_map<std::string, std::shared_ptr<_Resource>> resources;

    // Container of shared pointers to resources, allowing thread-safe access
    std::unordered_map<std::string, std::shared_ptr<boost::synchronized_value<_Resource*>>> owned_synchronized_resources;

    // Mutex for synchronizing access to the resources
    std::mutex mtx;

public:
    static ResourceManager& instance() {
        static ResourceManager instance;
        return instance;
    }

    std::unordered_set<std::string> getResourceIds() {
        std::lock_guard<std::mutex> lock(mtx);
        std::unordered_set<std::string> result;
        for (const auto& [id, _] : owned_synchronized_resources) {
            result.insert(id);
        }
        return result;
    }

    void create(const std::string_view _id, _Resource *resource) {
        std::println(stderr, "Creating resource: {}", _id);
        std::lock_guard lock(mtx);
        std::string id(_id);
        resources.insert_or_assign(id, std::shared_ptr<_Resource>(resource));
        auto r = std::make_shared<boost::synchronized_value<_Resource*>>(resources[id].get());
        owned_synchronized_resources.insert_or_assign(id, std::move(r));
    }

    void create(const std::string_view _id, std::shared_ptr<_Resource> &&resource) {
        std::println(stderr, "Creating resource: {}", _id);
        std::lock_guard lock(mtx);
        std::string id(_id);
        resources.insert_or_assign(id, std::move(resource));
        auto r = std::make_shared<boost::synchronized_value<_Resource*>>(resources[id].get());
        owned_synchronized_resources.insert_or_assign(id, std::move(r));
    }

    void destroy(const std::string_view _id) {
        std::println(stderr, "Destroying resource: {}", _id);
        std::lock_guard lock(mtx);
        std::string id(_id);
        auto synced_res = owned_synchronized_resources.erase(id);
        if (synced_res == 0) {
            std::println(stderr, "WARN: unable to delete synced resource {}", id);
        }
        auto res = resources.erase(id);
        if (res == 0) {
            std::println(stderr, "WARN: unable to delete resource {}", id);
        }
    }

    std::weak_ptr<Resource> getResource(const std::string_view _id) {
        std::string id(_id);
        std::lock_guard lock(mtx);
        auto it = owned_synchronized_resources.find(id);
        if (it == owned_synchronized_resources.end()) {
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

        for (const auto& [id, resPtr] : owned_synchronized_resources) {
            auto locked = resPtr->synchronize();
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
            auto it = owned_synchronized_resources.find(id);
            if (it == owned_synchronized_resources.end()) {
                std::println(stderr, "Resource with ID {} not found in registry, Skipping", id);
                continue;
            }

            auto locked = it->second->synchronize();
            (*locked)->deserialize(data);
        }
    }
};

// CRTP helper for automatic registration
template <typename Derived>
class RegisteredResource : public _Resource {
public:
    std::string_view getId() const override {
        return Derived::RESOURCE_ID;
    }

protected:
    RegisteredResource() = default;

    static void registerResource(Derived *res) {
        static bool registered = false;
        if (!registered) {
            registered = true;
            ResourceManager::instance().create(
                Derived::RESOURCE_ID,
                res
            );
        }
    }

    static void registerResource(std::shared_ptr<Derived> &&res) {
        static bool registered = false;
        if (!registered) {
            registered = true;
            ResourceManager::instance().create(
                Derived::RESOURCE_ID,
                std::move(res)
            );
        }
    }
};
