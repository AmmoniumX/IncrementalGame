#include "Recipes.hpp"

inline auto find_recipe(Recipes::RecipeSet &recipes, std::string_view id) {
  return std::find_if(recipes.begin(), recipes.end(),
                      [&id](const auto &recipe) {
                        auto [r_id, rec] = recipe;
                        return r_id == id;
                      });
}

void Recipes::add(std::string_view id, Recipes::Recipe recipe) {
  if (auto it = find_recipe(recipes, id); it != recipes.end()) {
    throw std::runtime_error("Duplicate recipe id");
  }
  recipes.emplace_back(std::string{id}, std::move(recipe));
}

std::optional<const Recipes::Recipe> Recipes::get(std::string_view id) {
  if (auto it = find_recipe(recipes, id); it != recipes.end()) {
    return it->second;
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
