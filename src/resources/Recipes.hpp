#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../systems/ResourceManager.hpp"
#include "./Inventory.hpp"

class Recipes : public detail::Resource {
    private:
    Recipes() = default;

    public:
    static inline const std::string RESOURCE_ID = "Recipes";
    struct Recipe {
        const std::string recipe_type;
        const std::vector<Inventory::ItemStack> inputs;
        const std::vector<Inventory::ItemStack> outputs;
    };

    // TODO store as JSON and implement recipe (de)serialization
    using Items = Inventory::Items;
    std::unordered_map<string, Recipe> recipes {
        {Items::IRON, {"crafting", {}, {{Items::IRON, 1}}}},
        {Items::COPPER, {"crafting", {}, {{Items::COPPER, 1}}}},
        {Items::IRON_GEAR, {"crafting", {{Items::IRON, 4}}, {{Items::IRON_GEAR, 1}}}},
        {Items::COPPER_WIRE, {"crafting", {{Items::COPPER, 1}}, {{Items::COPPER_WIRE, 3}}}},
        {Items::MOTOR, {"crafting", {{Items::IRON_GEAR, 2}, {Items::COPPER_WIRE, 10}}, {{Items::MOTOR, 1}}}}
    };

    void add(std::string id, Recipe recipe);

    std::optional<const Recipe> get(std::string id);

    static void init();

    std::optional<json> serialize() const override;

    void deserialize([[maybe_unused]] const json& j) override;

};
