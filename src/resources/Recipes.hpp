#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "./SaveData.hpp"

using namespace Save;

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
  using ItemStack = SaveData::ItemStack;
  std::unordered_map<std::string, Recipe> recipes{
      {std::string(Items::IRON),
       Recipe{"crafting", {}, {ItemStack{Items::IRON, 1}}}},
      {std::string(Items::COPPER),
       Recipe{"crafting", {}, {ItemStack{Items::COPPER, 1}}}},
      {std::string(Items::IRON_GEAR), Recipe{"crafting",
                                             {ItemStack{Items::IRON, 4}},
                                             {ItemStack{Items::IRON_GEAR, 1}}}},
      {std::string(Items::COPPER_WIRE),
       Recipe{"crafting",
              {ItemStack{Items::COPPER, 1}},
              {ItemStack{Items::COPPER_WIRE, 3}}}},
      {std::string(Items::MOTOR), Recipe{"crafting",
                                         {ItemStack{Items::IRON_GEAR, 2},
                                          ItemStack{Items::COPPER_WIRE, 10}},
                                         {ItemStack{Items::MOTOR, 1}}}}};

  void add(std::string_view id, Recipe recipe);

  std::optional<const Recipe> get(std::string_view id);

  static void init();

  json serialize() const;

  void deserialize(const json &j);
};
