#pragma once

#include <map>
#include <string>
#include <string_view>
#include <fstream>

#include "../game.hpp"

class SaveData {
  private:
    std::map<std::string, BigNum> items;
    SaveData() = default;
    
    void fromJson(const json);
    json toJson() const;

  public:

    static SaveData &instance() {
        static SaveData instance;
        return instance;
    }

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

    const std::map<std::string, BigNum> &getItems() const;

    BigNum getItem(const std::string_view _item) const;

    void setItem(const std::string_view _item, const BigNum &amount);

    void addItem(const std::string_view _item, const BigNum &amount);

    void subtractItem(const std::string_view _item, const BigNum &amount);

    void serialize(std::ofstream &file) const;

    void deserialize(std::ifstream &file);

    // Prevent copying and assignment
    SaveData(const SaveData &) = delete;
    SaveData &operator=(const SaveData &) = delete;
    ~SaveData() = default;
};
