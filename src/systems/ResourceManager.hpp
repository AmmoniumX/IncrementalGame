#pragma once

#include <optional>
#include <print>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include "../json.hpp"
#include "../SystemManager.hpp"

using nlohmann::json;

class Resource {
public:
    virtual ~Resource() = default;

    virtual std::optional<json> serialize() const;
    virtual void deserialize([[maybe_unused]] const json& j);

    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

protected:
    Resource() = default;
};

class ResourceManager : public System {
private:
    ResourceManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;

public:

    static void init();

    static ResourceManager& instance();

    std::unordered_set<std::string> getResourceIds();

    void create(std::string id, Resource *resource);

    void create(std::string id, std::shared_ptr<Resource> &&resource);

    void destroy(std::string id);

    // Throws std::runtime_error on resource not found
    std::shared_ptr<Resource> getResource(std::string id);

    std::weak_ptr<Resource> getResourceWeak(std::string id);

    std::optional<std::shared_ptr<Resource>> getResourceOptional(std::string id);

    json serialize();

    void deserialize(const json& j);
};
