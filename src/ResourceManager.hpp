#pragma once

#include <iostream>
#include <print>
#include <format>
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
    virtual std::string getId() const = 0;
    virtual void onTick(const uint& gameTick) = 0;

    _Resource(const _Resource&) = delete;
    _Resource& operator=(const _Resource&) = delete;

protected:
    _Resource() = default;
};

using Resource = boost::synchronized_value<_Resource*>;
class _ResourceManager {
private:
    _ResourceManager() = default;

    // Container of uniquely owned resources
    std::unordered_map<std::string, std::unique_ptr<_Resource>> owned_resources;

    // Container of shared pointers to resources, allowing thread-safe access
    std::unordered_map<std::string, std::unique_ptr<boost::synchronized_value<_Resource*>>> owned_synchronized_resources;

    // Mutex for synchronizing access to the resources
    std::mutex mtx_resources;

public:
    static _ResourceManager& create() {
        static _ResourceManager instance;
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

    void addResource(const std::string& id, std::unique_ptr<_Resource>&& moved_resource) {
        std::println(std::cerr, "Registering resource: {}", id);
        std::lock_guard<std::mutex> lock(mtx_resources);
        owned_resources.insert_or_assign(id, std::move(moved_resource));
        auto resource = std::make_unique<boost::synchronized_value<_Resource*>>(owned_resources[id].get());
        owned_synchronized_resources.insert_or_assign(id, std::move(resource));
    }

    // This returns a pointer to a synchronized value that contains the resource.
    // The internal pointer is a "raw" pointer, which is safe to use as long as the resource is not deleted.
    // This is safe because the ResourceManager owns the resource and guarantees its lifetime, and
    // ResourceManager itself is only destroyed at the end of the program.
    Resource &getResource(const std::string& resourceId) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        auto it = owned_synchronized_resources.find(resourceId);
        if (it == owned_synchronized_resources.end()) {
            throw std::runtime_error(std::format("Invalid resource ID: {}", resourceId));
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

// Inline definition of global instance
inline _ResourceManager& ResourceManager = _ResourceManager::create();

// CRTP helper for automatic registration
template <typename Derived>
class RegisteredResource : public _Resource {
public:
    std::string getId() const override {
        return Derived::RESOURCE_ID;
    }

protected:
    RegisteredResource() = default;

    static void registerResource(std::unique_ptr<Derived> &&instance) {
        static bool registered = false;
        if (!registered) {
            registered = true;
            ResourceManager.addResource(
                Derived::RESOURCE_ID,
                std::move(instance)
            );
        }
    }
};
