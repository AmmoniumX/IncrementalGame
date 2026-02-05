#include <fstream>

#include "../../include/json.hpp"
#include "../Logger.hpp"
#include "SaveData.hpp"

using namespace Save;

const SaveData::Map &SaveData::getItems() const { return items; }

BigNum SaveData::getItem(const std::string_view id) const {
  if (auto it = items.find(id); it != items.end()) {
    return it->second;
  }
  return BigNum(0); // Return 0 if item not found
}

void SaveData::setItem(const std::string_view id, const BigNum &amount) {
  items.insert_or_assign(std::string(id), amount);
}

void SaveData::addItem(const std::string_view id, const BigNum &amount) {
  if (auto it = items.find(id); it != items.end()) {
    it->second += amount; // Fix: Access .second
    return;
  }
  items.emplace(
      id, amount); // Fix: Use emplace for string_view to string conversion
}

void SaveData::subtractItem(const std::string_view id, const BigNum &amount) {
  if (auto it = items.find(id); it != items.end()) {
    it->second -= amount;
    if (it->second < 0)
      it->second = 0;
  } else {
    items.emplace(id, 0);
  }
}

const SaveData::Map &SaveData::getUpgrades() const { return upgrades; }

BigNum SaveData::getUpgradeLvl(const std::string_view id) const {
  if (auto it = upgrades.find(id); it != upgrades.end()) {
    return it->second;
  }
  return BigNum(0); // Return 0 if item not found
}

void SaveData::addUpgradeLvl(const std::string_view id, const BigNum &lvl) {
  if (auto it = upgrades.find(id); it != upgrades.end()) {
    it->second += lvl;
  } else {
    upgrades.emplace(id, lvl);
  }
}

void saveCategory(json &j, const std::string &category,
                  const SaveData::Map &map) {
  auto &j_category = j[category] = json::object();

  for (const auto &item : map) {
    j_category[item.first] = item.second.serialize();
  }
}

json SaveData::toJson() const {
  json j = json::object();
  saveCategory(j, "items", items);
  saveCategory(j, "upgrades", upgrades);
  return j;
}

void readCategory(const json &j, const std::string &category,
                  SaveData::Map &map) {
  if (j.contains(category) && j[category].is_object()) {
    const auto &j_category = j[category];

    for (auto it = j_category.begin(); it != j_category.end(); ++it) {
      std::string key = it.key();
      auto &value = it.value();

      std::string valueStr = value.is_string() ? value.get<std::string>() : "0";
      map[key] = BigNum::deserialize(valueStr);
    }
  }
}

void SaveData::fromJson(const json &j) {
  readCategory(j, "items", items);
  readCategory(j, "upgrades", upgrades);
}

void SaveData::serialize(std::ofstream &file) const {
  json j = SaveData::toJson();
  file << j.dump() << std::endl;
}

void SaveData::deserialize(std::ifstream &file) {
  json j;
  try {
    file >> j;
  } catch (const std::exception &e) {
    Logger::println("Error: Could not parse json! Is data corrupted? {}",
                    e.what());
    std::exit(EXIT_FAILURE);
  }

  SaveData::fromJson(j);
}
