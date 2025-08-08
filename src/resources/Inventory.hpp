#pragma once

#include <map>
#include <string>
#include <string_view>

#include "../ResourceManager.hpp"
#include "../game.hpp"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;
class Inventory : public RegisteredResource<Inventory> {
  private:
    std::map<std::string, BigNum> items;

  public:
    static constexpr std::string_view RESOURCE_ID = "inventory"sv;

    struct Items {
        static constexpr std::string_view IRON = "Iron"sv;
        static constexpr std::string_view COPPER = "Copper"sv;
        static constexpr std::string_view IRON_GEAR = "Iron Gear"sv;
        static constexpr std::string_view COPPER_WIRE = "Copper Wire"sv;
        static constexpr std::string_view MOTOR = "Motor"sv;
    };

    struct ItemStack {
        const std::string_view id;
        const BigNum amount;
        constexpr ItemStack(const std::string_view id, const BigNum amount) : id(id), amount(amount) {}
    };


    Inventory() {}

    static void create() {
        static std::unique_ptr<Inventory> instance =
            std::make_unique<Inventory>();
        registerResource(std::move(instance));
    }

    const std::map<std::string, BigNum> &getItems() const { return items; }

    BigNum getItem(const std::string_view _item) const {
        std::string item(_item);
        auto it = items.find(std::string(item));
        if (it != items.end()) {
            return it->second;
        }
        return BigNum(0); // Return 0 if item not found
    }

    void setItem(const std::string_view _item, const BigNum &amount) {
        std::string item(_item);
        items[item] = amount;
    }

    void addItem(const std::string_view _item, const BigNum &amount) {
        std::string item(_item);
        items[item] += amount;
    }

    void subtractItem(const std::string_view _item, const BigNum &amount) {
        std::string item(_item);
        items[item] -= amount;
        if (items[item] < BigNum(0)) {
            items[item] = BigNum(0); // Prevent negative amounts
        }
    }

    virtual json serialize() const override {
        json j;
        for (const auto &item : items) {
            j[item.first] = item.second.serialize();
        }
        return j;
    }

    virtual void deserialize(const json &j) override {
        for (const auto &[key, value] : j.items()) {
            std::string valueStr =
                value.is_string() ? value.get<std::string>() : "0";
            items[key] = BigNum::deserialize(valueStr);
        }
    }

    virtual void onTick([[maybe_unused]] const uint &gameTick) override {
        // No specific logic
    }

    // Prevent copying and assignment
    Inventory(const Inventory &) = delete;
    Inventory &operator=(const Inventory &) = delete;
};
