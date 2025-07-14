#pragma once

#include <iostream>
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

class _ResourceRegistry {
private:
    _ResourceRegistry() = default;

    using ThreadSafeResource = boost::synchronized_value<std::shared_ptr<Resource>>;
    using ResourcePtr = std::shared_ptr<ThreadSafeResource>;

    std::unordered_map<std::string, ResourcePtr> resources;
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

    void addResource(const std::string& id, std::shared_ptr<Resource>&& moved_resource) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        auto resource = std::make_shared<ThreadSafeResource>(std::shared_ptr<Resource>(std::move(moved_resource)));
        resources.insert_or_assign(id, std::move(resource));
    }

    template <typename T = Resource>
    std::shared_ptr<T> getResource(const std::string& resourceId) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        auto it = resources.find(resourceId);
        if (it == resources.end()) return nullptr;

        auto locked = it->second->synchronize();
        return std::dynamic_pointer_cast<T>(*locked); // Cast from shared_ptr<Resource> to T
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

    static void registerResource() {
        static bool registered = false;
        if (!registered) {
            registered = true;
            ResourceRegistry.addResource(
                Derived::RESOURCE_ID,
                Derived::getInstance()
            );
        }
    }
};
