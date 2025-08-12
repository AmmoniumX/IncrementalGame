#include "ResourceManager.hpp"
#include "../resources/Inventory.hpp"
#include "../resources/Recipes.hpp"

void ResourceManager::init() {
    std::println(stderr, "Registering resources...");
    Inventory::init();
    Recipes::init();
}

std::optional<json> detail::Resource::serialize() const {
    return std::nullopt; 
};

void detail::Resource::deserialize([[maybe_unused]] const json& j) {};

std::shared_ptr<Resource> ResourceManager::newResource(std::unique_ptr<detail::Resource> &&r) {
    return std::make_shared<Resource>(boost::synchronized_value<std::unique_ptr<detail::Resource>>(std::move(r)));
}

std::shared_ptr<Resource> ResourceManager::newResource(detail::Resource *r) {
    return std::make_shared<Resource>(boost::synchronized_value<std::unique_ptr<detail::Resource>>(std::unique_ptr<detail::Resource>(r)));
}

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance;
    return instance;
}

std::unordered_set<std::string> ResourceManager::getResourceIds() {
    std::lock_guard<std::mutex> lock(mtx);
    std::unordered_set<std::string> result;
    for (const auto& [id, _] : resources) {
        result.insert(id);
    }
    return result;
}

void ResourceManager::create(std::string id, detail::Resource *resource) {
    std::println(stderr, "Creating resource: {}", id);
    std::lock_guard lock(mtx);
    resources.insert_or_assign(id, newResource(resource));
}

void ResourceManager::create(std::string id, std::unique_ptr<detail::Resource> &&resource) {
    std::println(stderr, "Creating resource: {}", id);
    std::lock_guard lock(mtx);
    resources.insert_or_assign(id, newResource(std::move(resource)));
}

void ResourceManager::destroy(std::string id) {
    std::println(stderr, "Destroying resource: {}", id);
    std::lock_guard lock(mtx);
    auto res = resources.erase(id);
    if (res == 0) {
        std::println(stderr, "WARN: unable to delete resource {}", id);
    }
}

std::shared_ptr<Resource> ResourceManager::getResource(std::string id) {
    using namespace std::literals::string_literals;
    std::lock_guard lock(mtx);
    if (auto r = resources.find(id); r != resources.end()) {
        return r->second;
    }
    throw std::runtime_error("Resource not found: "s + id);
}

std::weak_ptr<Resource> ResourceManager::getResourceWeak(std::string id) {
    std::lock_guard lock(mtx);
    if (auto r = resources.find(id); r != resources.end()) {
        return r->second;
    }
    return std::weak_ptr<Resource>{}; // return empty ptr
}

std::optional<std::shared_ptr<Resource>> ResourceManager::getResourceOptional(std::string id) {
    using namespace std::literals::string_literals;
    std::lock_guard lock(mtx);
    if (auto r = resources.find(id); r != resources.end()) {
        return r->second;
    }
    return std::nullopt;
}

json ResourceManager::serialize() {
    std::lock_guard lock(mtx);
    json result = json::object();

    for (const auto& [id, res] : resources) {
        auto locked = res->synchronize();
        auto serialized = (*locked)->serialize();
        if (serialized) { result[id] = *serialized; } // only serialize needed resources
    }

    return json{{"resources", result}};
}

void ResourceManager::deserialize(const json& j) {
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

