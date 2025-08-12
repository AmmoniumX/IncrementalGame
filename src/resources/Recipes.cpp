#include "Recipes.hpp"

void Recipes::add(std::string id, Recipes::Recipe recipe) {
    if (recipes.contains(id)) { throw std::runtime_error("Duplicate recipe id"); }
    recipes.emplace(std::move(id), std::move(recipe));
}

std::optional<const Recipes::Recipe> Recipes::get(std::string id) {
    if (auto r = recipes.find(id); r != recipes.end()) {
        return r->second;
    }

    return std::nullopt;
}

void Recipes::init() {
    static bool registered = false;
    if (!registered) {
        ResourceManager::instance().create(RESOURCE_ID, std::unique_ptr<Recipes>(new Recipes));
        registered = true;
    }
}

std::optional<json> Recipes::serialize() const {
    // TODO json recipe serialization
    return std::nullopt;
}

void Recipes::deserialize([[maybe_unused]] const json& j) {
    // TODO json recipe deserialization
}
