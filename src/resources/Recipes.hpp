#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "./SaveData.hpp"

class Recipes {
private:
  Recipes() = default;

public:
  struct Recipe {
    const std::string recipe_type;
    const std::vector<SaveData::ItemStack> inputs;
    const std::vector<SaveData::ItemStack> outputs;
  };

  static Recipes &instance() {
    static Recipes instance;
    return instance;
  }

  // TODO store as JSON and implement recipe (de)serialization
  using Items = SaveData::Items;
  std::unordered_map<string, Recipe> recipes{
      {Items::IRON, {"crafting", {}, {{Items::IRON, 1}}}},
      {Items::COPPER, {"crafting", {}, {{Items::COPPER, 1}}}},
      {Items::IRON_GEAR,
       {"crafting", {{Items::IRON, 4}}, {{Items::IRON_GEAR, 1}}}},
      {Items::COPPER_WIRE,
       {"crafting", {{Items::COPPER, 1}}, {{Items::COPPER_WIRE, 3}}}},
      {Items::MOTOR,
       {"crafting",
        {{Items::IRON_GEAR, 2}, {Items::COPPER_WIRE, 10}},
        {{Items::MOTOR, 1}}}}};

  void add(std::string id, Recipe recipe);

  std::optional<const Recipe> get(std::string id);

  static void init();

  json serialize() const;

  void deserialize(const json &j);
};
