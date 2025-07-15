#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <boost/thread/synchronized_value.hpp>

#include "./json.hpp"
#include "./BigNum.hpp"

using nlohmann::json;

class Resource {
public:
    virtual ~Resource() = default;

    virtual json serialize() const = 0;
    virtual void deserialize(const json& j) = 0;
    virtual std::string getId() const = 0;
    virtual void onTick(const uint& gameTick) = 0;

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

protected:
    Resource() = default;
};

using ResourcePtr = std::shared_ptr<boost::synchronized_value<Resource*>>;
class _ResourceRegistry {
private:
    _ResourceRegistry() = default;

    // Container of uniquely owned resources
    std::unordered_map<std::string, std::unique_ptr<Resource>> owned_resources;

    // Container of shared pointers to resources, allowing thread-safe access
    std::unordered_map<std::string, ResourcePtr> resources;

    // Mutex for synchronizing access to the resources
    std::mutex mtx_resources;

public:
    static _ResourceRegistry& getInstance() {
        static _ResourceRegistry instance;
        return instance;
    }

    std::unordered_set<std::string> getResourceIds() {
        std::lock_guard<std::mutex> lock(mtx_resources);
        std::unordered_set<std::string> result;
        for (const auto& [id, _] : resources) {
            result.insert(id);
        }
        return result;
    }

    void addResource(const std::string& id, std::unique_ptr<Resource>&& moved_resource) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        owned_resources.insert_or_assign(id, std::move(moved_resource));
        auto resource = std::make_shared<boost::synchronized_value<Resource*>>(owned_resources[id].get());
        resources.insert_or_assign(id, std::move(resource));
    }

    // This returns a pointer to a synchronized value that contains the resource.
    // The internal pointer is a "raw" pointer, which is safe to use as long as the resource is not deleted.
    // This is safe because the ResourceRegistry owns the resource and guarantees its lifetime, and
    // ResourceRegistry itself is only destroyed at the end of the program.
    ResourcePtr getResource(const std::string& resourceId) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        auto it = resources.find(resourceId);
        if (it == resources.end()) {
            return nullptr;
        }

        // We return the shared_ptr to the resource.
        // The caller will then call synchronize() on this returned object.
        return it->second;
    }

    json serialize() {
        std::lock_guard<std::mutex> lock(mtx_resources);
        json result = json::object();

        for (const auto& [id, resPtr] : resources) {
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
            auto it = resources.find(id);
            if (it == resources.end()) {
                std::cerr << "Resource with ID '" << id << "' not found in registry. Skipping deserialization.\n";
                continue;
            }

            auto locked = it->second->synchronize();
            (*locked)->deserialize(data);
        }
    }

    void onTick(const uint& gameTick) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        for (const auto& [_, resPtr] : resources) {
            auto locked = resPtr->synchronize();
            (*locked)->onTick(gameTick);
        }
    }
};

// Inline definition of global instance
inline _ResourceRegistry& ResourceRegistry = _ResourceRegistry::getInstance();

// CRTP helper for automatic registration
template <typename Derived>
class RegisteredResource : public Resource {
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
            ResourceRegistry.addResource(
                Derived::RESOURCE_ID,
                std::move(instance)
            );
        }
    }
};