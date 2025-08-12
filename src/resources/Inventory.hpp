#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "../game.hpp"
#include "../systems/ResourceManager.hpp"

using namespace std::literals::string_literals;

class Inventory : public detail::Resource {
  private:
    std::map<std::string, BigNum> items;
    Inventory() {}
    
  public:
    static inline const std::string RESOURCE_ID = "Inventory"s;

    struct Items {
        static inline const std::string IRON = "Iron"s;
        static inline const std::string COPPER = "Copper"s;
        static inline const std::string IRON_GEAR = "Iron Gear"s;
        static inline const std::string COPPER_WIRE = "Copper Wire"s;
        static inline const std::string MOTOR = "Motor"s;
    };

    struct ItemStack {
        const std::string id;
        const BigNum amount;
    };

    static void init() {
        // std::println(stderr, "Registering inventory...");
        static bool registered = false;
        if (!registered) {
            ResourceManager::instance().create(RESOURCE_ID, std::unique_ptr<Inventory>(new Inventory));
            registered = true;
        }
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

    std::optional<json> serialize() const override {
        json j = json::object();
        for (const auto &item : items) {
            j[item.first] = item.second.serialize();
        }
        return j;
    }

    void deserialize(const json &j) override {
        for (const auto &[key, value] : j.items()) {
            std::string valueStr =
                value.is_string() ? value.get<std::string>() : "0";
            items[key] = BigNum::deserialize(valueStr);
        }
    }

    // Prevent copying and assignment
    Inventory(const Inventory &) = delete;
    Inventory &operator=(const Inventory &) = delete;
    ~Inventory() = default;
};
