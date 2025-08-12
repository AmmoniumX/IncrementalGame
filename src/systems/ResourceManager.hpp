#pragma once

#include <optional>
#include <print>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>

#include <boost/thread/synchronized_value.hpp>

#include "../json.hpp"
#include "../SystemManager.hpp"

using nlohmann::json;

namespace detail{
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
} // namespace detail

using Resource = boost::synchronized_value<std::unique_ptr<detail::Resource>>;

class ResourceManager : public System {
private:
    ResourceManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
    std::mutex mtx;

    std::shared_ptr<Resource> newResource(std::unique_ptr<detail::Resource> &&r);

    std::shared_ptr<Resource> newResource(detail::Resource *r);
public:

    static void init();

    static ResourceManager& instance();

    std::unordered_set<std::string> getResourceIds();

    void create(std::string id, detail::Resource *resource);

    void create(std::string id, std::unique_ptr<detail::Resource> &&resource);

    void destroy(std::string id);

    // Throws std::runtime_error on resource not found
    std::shared_ptr<Resource> getResource(std::string id);

    std::weak_ptr<Resource> getResourceWeak(std::string id);

    std::optional<std::shared_ptr<Resource>> getResourceOptional(std::string id);

    json serialize();

    void deserialize(const json& j);
};
