#pragma once

#include <map>
#include <string>
#include <string_view>

#include "../game.hpp"
#include "../resourceRegistry.hpp"

class Inventory: public RegisteredResource<Inventory> {
private:
    std::map<std::string, BigNum> items;
public:
    static constexpr const std::string RESOURCE_ID = "inventory";

    struct Items {
        static constexpr const std::string IRON = "Iron";
        static constexpr const std::string COPPER = "Copper";
        static constexpr const std::string IRON_ROD = "Iron Rod";
        static constexpr const std::string COPPER_WIRE = "Copper Wire";
        static constexpr const std::string MOTOR = "Motor";
    };

    Inventory() {}

    static void create() {
        static std::unique_ptr<Inventory> instance = std::make_unique<Inventory>();
        registerResource(std::move(instance));
    }

    const std::map<std::string, BigNum>& getItems() const {
        return items;
    }

    BigNum getItem(const std::string& item) const {
        auto it = items.find(item);
        if (it != items.end()) {
            return it->second;
        }
        return BigNum(0); // Return 0 if item not found
    }

    void setItem(const std::string& item, const BigNum& amount) {
        items[item] = amount;
    }

    void addItem(const std::string& item, const BigNum& amount) {
        items[item] += amount;
    }

    void subtractItem(const std::string& item, const BigNum& amount) {
        items[item] -= amount;
        if (items[item] < BigNum(0)) {
            items[item] = BigNum(0); // Prevent negative amounts
        }
    }

    virtual json serialize() const override {
        json j;
        for (const auto& item : items) {
            j[item.first] = item.second.serialize();
        }
        return j;
    }

    virtual void deserialize(const json& j) override {
        for (const auto& [key, value] : j.items()) {
            std::string valueStr = value.is_string() ? value.get<std::string>() : "0";
            items[key] = BigNum::deserialize(valueStr);
        }
    }

    virtual void onTick([[maybe_unused]] const uint &gameTick) override {
        // No specific logic
    }

    // Prevent copying and assignment
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

};