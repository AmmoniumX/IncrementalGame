#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "./SaveData.hpp"

using namespace Save;

class Recipes {
private:
  Recipes() = default;
  using ItemStack = SaveData::ItemStack;

public:
  struct Recipe {
    std::string recipe_type;
    std::vector<SaveData::ItemStack> inputs;
    std::vector<SaveData::ItemStack> outputs;

    Recipe(std::string_view rt, std::vector<SaveData::ItemStack> in = {},
           std::vector<SaveData::ItemStack> out = {})
        : recipe_type{rt}, inputs{in}, outputs{out} {}
  };

  static Recipes &instance() {
    static Recipes instance;
    return instance;
  }

  // TODO store as JSON and implement recipe (de)serialization
  using RecipeSet = std::vector<std::pair<std::string, Recipe>>;
  RecipeSet recipes{
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
                                         {ItemStack{Items::MOTOR, 1}}}},
      {"MOTOR_BILLS", Recipe{"crafting",
                             {ItemStack{Items::MOTOR, 1}},
                             {ItemStack{Items::BILLS, 20}}}}};

  const RecipeSet &getRecipes() { return recipes; }

  void add(std::string_view id, Recipe recipe);

  std::optional<const Recipe> get(std::string_view id);

  static void init();

  json serialize() const;

  void deserialize(const json &j);
};
