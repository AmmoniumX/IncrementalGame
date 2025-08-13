#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

#include "../game.hpp"
#include "../systems/ResourceManager.hpp"

class Inventory : public Resource {
  private:
    std::map<std::string, BigNum> items;
    Inventory() = default;
    
  public:
    static inline const std::string RESOURCE_ID = "Inventory";

    struct Items {
        static inline const std::string IRON = "Iron";
        static inline const std::string COPPER = "Copper";
        static inline const std::string IRON_GEAR = "Iron Gear";
        static inline const std::string COPPER_WIRE = "Copper Wire";
        static inline const std::string MOTOR = "Motor";
    };

    struct ItemStack {
        const std::string id;
        const BigNum amount;
    };

    static void init();

    const std::map<std::string, BigNum> &getItems() const;

    BigNum getItem(const std::string_view _item) const;

    void setItem(const std::string_view _item, const BigNum &amount);

    void addItem(const std::string_view _item, const BigNum &amount);

    void subtractItem(const std::string_view _item, const BigNum &amount);

    std::optional<json> serialize() const override;

    void deserialize(const json &j) override;

    // Prevent copying and assignment
    Inventory(const Inventory &) = delete;
    Inventory &operator=(const Inventory &) = delete;
    ~Inventory() = default;
};
