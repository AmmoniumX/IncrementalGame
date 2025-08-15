#include "../Logger.hpp"
#include "../game.hpp"
#include "ResourceManager.hpp"
#include "../resources/Inventory.hpp"
#include "../resources/Recipes.hpp"

void ResourceManager::init() {
    Logger::println("Registering resources...");
    Inventory::init();
    Recipes::init();
}

std::optional<json> Resource::serialize() const {
    return std::nullopt; 
};

void Resource::deserialize([[maybe_unused]] const json& j) {};

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance;
    return instance;
}

std::unordered_set<std::string> ResourceManager::getResourceIds() {
    std::unordered_set<std::string> result;
    for (const auto& [id, _] : resources) {
        result.insert(id);
    }
    return result;
}

void ResourceManager::create(std::string id, Resource *resource) {
    Logger::println("Creating resource: {}", id);
    resources.insert_or_assign(id, std::unique_ptr<Resource>(resource));
}

void ResourceManager::create(std::string id, std::shared_ptr<Resource> &&resource) {
    Logger::println("Creating resource: {}", id);
    resources.insert_or_assign(id, std::move(resource));
}

void ResourceManager::destroy(std::string id) {
    Logger::println("Destroying resource: {}", id);
    auto res = resources.erase(id);
    if (res == 0) {
        Logger::println("WARN: unable to delete resource {}", id);
    }
}

std::shared_ptr<Resource> ResourceManager::getResource(std::string id) {
    using namespace std::literals::string_literals;
    if (auto r = resources.find(id); r != resources.end()) {
        return r->second;
    }
    throw std::runtime_error("Resource not found: "s + id);
}

std::weak_ptr<Resource> ResourceManager::getResourceWeak(std::string id) {
    if (auto r = resources.find(id); r != resources.end()) {
        return r->second;
    }
    return std::weak_ptr<Resource>{}; // return empty ptr
}

std::optional<std::shared_ptr<Resource>> ResourceManager::getResourceOptional(std::string id) {
    using namespace std::literals::string_literals;
    if (auto r = resources.find(id); r != resources.end()) {
        return r->second;
    }
    return std::nullopt;
}

json ResourceManager::serialize() {
    json result = json::object();

    for (const auto& [id, res] : resources) {
        auto serialized = res->serialize();
        if (serialized) { result[id] = *serialized; } // only serialize needed resources
    }

    return json{{"resources", result}};
}

void ResourceManager::deserialize(const json& j) {
    if (!j.contains("resources")) return;

    const auto& res_json = j.at("resources");

    for (const auto& [id, data] : res_json.items()) {
        auto it = resources.find(id);
        if (it == resources.end()) {
            Logger::println("Resource with ID {} not found in registry, Skipping", id);
            continue;
        }

        it->second->deserialize(data);
    }
}

