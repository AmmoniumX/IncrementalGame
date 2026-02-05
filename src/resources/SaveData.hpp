#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../game.hpp"

using namespace std::string_view_literals;

// A custom hash function object that can handle std::string, std::string_view,
// and const char*
struct StringHash {
  // This using declaration enables the heterogeneous lookup
  using is_transparent = void;

  [[nodiscard]] size_t operator()(std::string_view txt) const {
    return std::hash<std::string_view>{}(txt);
  }
  [[nodiscard]] size_t operator()(const std::string &txt) const {
    return std::hash<std::string>{}(txt);
  }
  [[nodiscard]] size_t operator()(const char *txt) const {
    return std::hash<std::string_view>{}(
        txt); // use string_view hash for const char*
  }
};

class SaveData {
public:
  using Map =
      std::unordered_map<std::string, BigNum, StringHash, std::equal_to<>>;

private:
  Map items{};
  Map upgrades{};
  SaveData() = default;

  void fromJson(const json &);
  json toJson() const;

public:
  static SaveData &instance() {
    static SaveData instance;
    return instance;
  }

  // List of all Items
  struct Items {
    static constexpr auto IRON = "Iron"sv;
    static constexpr auto COPPER = "Copper"sv;
    static constexpr auto IRON_GEAR = "Iron Gear"sv;
    static constexpr auto COPPER_WIRE = "Copper Wire"sv;
    static constexpr auto MOTOR = "Motor"sv;
  };

  struct ItemStack {
    std::string id;
    BigNum amount;

    ItemStack(std::string id, BigNum amount) : id{id}, amount{amount} {};
    ItemStack(std::string_view id, BigNum amount) : id{id}, amount{amount} {};
  };

  struct Upgrades {
    static constexpr auto DOUBLE_RAW_PRODUCTION = "DoubleRawProduction"sv;
  };

  struct Upgrade {
    std::string id;
    BigNum lvl;
  };

  const Map &getItems() const;

  BigNum getItem(const std::string_view id) const;

  void setItem(const std::string_view id, const BigNum &amount);

  void addItem(const std::string_view id, const BigNum &amount);

  void subtractItem(const std::string_view id, const BigNum &amount);

  const Map &getUpgrades() const;

  BigNum getUpgradeLvl(const std::string_view id) const;

  void setUpgradeLvl(const std::string_view id, const BigNum &lvl);

  void addUpgradeLvl(const std::string_view id, const BigNum &lvl);

  void serialize(std::ofstream &file) const;

  void deserialize(std::ifstream &file);

  // Prevent copying and assignment
  SaveData(const SaveData &) = delete;
  SaveData &operator=(const SaveData &) = delete;
  ~SaveData() = default;
};
