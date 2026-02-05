#include "Recipes.hpp"

void Recipes::add(std::string_view id, Recipes::Recipe recipe) {
  if (recipes.contains(std::string{id})) {
    throw std::runtime_error("Duplicate recipe id");
  }
  recipes.emplace(std::move(id), std::move(recipe));
}

std::optional<const Recipes::Recipe> Recipes::get(std::string_view id) {
  if (auto r = recipes.find(std::string{id}); r != recipes.end()) {
    return r->second;
  }

  return std::nullopt;
}

json Recipes::serialize() const {
  // TODO json recipe serialization
  return json();
}

void Recipes::deserialize(const json &j) {
  // TODO json recipe deserialization
  (void)j;
}
