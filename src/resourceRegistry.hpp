#pragma once

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "./json.hpp"
#include "./BigNum.hpp"

using nlohmann::json;

class Resource {
public:
    // Virtual destructor is important for proper cleanup in inheritance
    virtual ~Resource() = default;
    
    // Virtual functions
    virtual json serialize() const = 0;
    virtual void deserialize(const json& j) = 0;
    virtual std::string getId() const = 0;
    virtual void onTick(const uint &gameTick) = 0;
    
    // Prevent copying and assignment
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    
protected:
    // Protected constructor prevents direct instantiation
    Resource() {};
};
    
class _ResourceRegistry {
private:
    _ResourceRegistry() = default;
    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
    std::mutex mtx_resources;
public:
    // Singleton access
    static _ResourceRegistry& getInstance() {
        static _ResourceRegistry instance;
        return instance;
    }

    std::unordered_set<std::string> getResourceIds() {
        std::lock_guard<std::mutex> lock(mtx_resources);
        std::unordered_set<std::string> result;
        for (const auto& [id, resource] : resources) {
            result.insert(id);
        }
        return result;
    }
    
    // Register a new resource type
    void addResource(const std::string& resourceId, std::shared_ptr<Resource> resource) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        resources[resourceId] = resource;
    }
    
    // Get a resource by ID
    template <typename T>
    std::shared_ptr<T> getResource(const std::string& resourceId) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        auto resource = resources[resourceId];
        if (resource) {
            return std::dynamic_pointer_cast<T>(resource);
        }
        return nullptr;
    }
    
    // Serialize all resources into a nested JSON structure
    json serialize() {
        std::lock_guard<std::mutex> lock_resources(mtx_resources);
        
        json resources_j = json::object({});
        // For each resource, serialize it and add to the result
        for (const auto& [id, resource] : resources) {
            resources_j[id] = resource->serialize();
        }
        
        // Return resources
        return json{
            {"resources", resources_j}
        };
    }
    
    // Deserialize resources from a nested JSON structure
    void deserialize(const json& j) {

        // Deserialize resources
        if (j.contains("resources")) {
            json resources_j = j.at("resources");
            // For each resource in the JSON, deserialize it and store it
            for (const auto& [id, data] : resources_j.items()) {
                auto resource = getResource<Resource>(id);
                if (resource) {
                    resource->deserialize(data);
                }
            }
        }
    }

    void onTick(const uint &gameTick) {
        // std::lock_guard<std::mutex> lock(mtx_resources);
        for (const auto& [id, resource] : resources) {
            resource->onTick(gameTick);
        }
    }
};

extern _ResourceRegistry& ResourceRegistry;

// CRTP helper for automatic registration
template <typename Derived>
class RegisteredResource : public Resource {
public:
    // Implementation of getId() using the CRTP pattern
    std::string getId() const override {
        return Derived::RESOURCE_ID;
    }
    
protected:
    RegisteredResource() = default;
    
    // Register the resource when used for the first time
    static void registerResource() {
        static bool registered = false;
        if (!registered) {
            registered = true;
            _ResourceRegistry::getInstance().addResource(
                Derived::RESOURCE_ID,
                Derived::getInstance()
            );
        }
    }
};
