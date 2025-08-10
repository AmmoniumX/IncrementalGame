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
    virtual void onTick(const uint& gameTick) = 0;

    _Resource(const _Resource&) = delete;
    _Resource& operator=(const _Resource&) = delete;

protected:
    _Resource() = default;
};

using Resource = boost::synchronized_value<_Resource*>;
class ResourceManager {
private:
    ResourceManager() = default;

    // Container of uniquely owned resources
    std::unordered_map<std::string, std::unique_ptr<_Resource>> owned_resources;

    // Container of shared pointers to resources, allowing thread-safe access
    std::unordered_map<std::string, std::unique_ptr<boost::synchronized_value<_Resource*>>> owned_synchronized_resources;

    // Mutex for synchronizing access to the resources
    std::mutex mtx_resources;

public:
    static ResourceManager& instance() {
        static ResourceManager instance;
        return instance;
    }

    std::unordered_set<std::string> getResourceIds() {
        std::lock_guard<std::mutex> lock(mtx_resources);
        std::unordered_set<std::string> result;
        for (const auto& [id, _] : owned_synchronized_resources) {
            result.insert(id);
        }
        return result;
    }

    void create(const std::string_view _id, std::unique_ptr<_Resource>&& moved_resource) {
        std::println(stderr, "Creating resource: {}", _id);
        std::lock_guard<std::mutex> lock(mtx_resources);
        std::string id(_id);
        owned_resources.insert_or_assign(id, std::move(moved_resource));
        auto resource = std::make_unique<boost::synchronized_value<_Resource*>>(owned_resources[id].get());
        owned_synchronized_resources.insert_or_assign(id, std::move(resource));
    }

    void destroy(const std::string_view _id) {
        std::println(stderr, "Destroying resource: {}", _id);
        std::lock_guard<std::mutex> lock(mtx_resources);
        std::string id(_id);
        auto synced_res = owned_synchronized_resources.erase(id);
        if (synced_res == 0) {
            std::println(stderr, "WARN: unable to delete synced resource {}", id);
        }
        auto res = owned_resources.erase(id);
        if (res == 0) {
            std::println(stderr, "WARN: unable to delete resource {}", id);
        }
    }

    std::optional<std::reference_wrapper<Resource>> getResource(const std::string_view resourceId) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        auto it = owned_synchronized_resources.find(std::string(resourceId));
        if (it == owned_synchronized_resources.end()) {
            // Resource either doesn't exist or was removed
            return std::nullopt;
        }

        // We return the pointer to the resource.
        // The caller will then call synchronize() on this returned object.
        return *it->second.get();
    }

    json serialize() {
        std::lock_guard<std::mutex> lock(mtx_resources);
        json result = json::object();

        for (const auto& [id, resPtr] : owned_synchronized_resources) {
            auto locked = resPtr->synchronize();
            result[id] = (*locked)->serialize();
        }

        return json{{"resources", result}};
    }

    void deserialize(const json& j) {
        if (!j.contains("resources")) return;

        const auto& res_json = j.at("resources");
        std::lock_guard<std::mutex> lock(mtx_resources);

        for (const auto& [id, data] : res_json.items()) {
            auto it = owned_synchronized_resources.find(id);
            if (it == owned_synchronized_resources.end()) {
                std::cerr << "Resource with ID '" << id << "' not found in registry. Skipping deserialization.\n";
                continue;
            }

            auto locked = it->second->synchronize();
            (*locked)->deserialize(data);
        }
    }

    void onTick(const uint& gameTick) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        for (const auto& [_, resPtr] : owned_synchronized_resources) {
            auto locked = resPtr->synchronize();
            (*locked)->onTick(gameTick);
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

    static void registerResource(std::unique_ptr<Derived> &&res) {
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
