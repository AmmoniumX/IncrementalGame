#include <fstream>

#include "../../include/json.hpp"
#include "../Logger.hpp"
#include "SaveData.hpp"

const std::map<std::string, BigNum> &SaveData::getItems() const {
  return items;
}

BigNum SaveData::getItem(const std::string_view _item) const {
  std::string item(_item);
  auto it = items.find(std::string(item));
  if (it != items.end()) {
    return it->second;
  }
  return BigNum(0); // Return 0 if item not found
}

void SaveData::setItem(const std::string_view _item, const BigNum &amount) {
  std::string item(_item);
  items[item] = amount;
}

void SaveData::addItem(const std::string_view _item, const BigNum &amount) {
  std::string item(_item);
  items[item] += amount;
}

void SaveData::subtractItem(const std::string_view _item,
                            const BigNum &amount) {
  std::string item(_item);
  items[item] -= amount;
  if (items[item] < BigNum(0)) {
    items[item] = BigNum(0); // Prevent negative amounts
  }
}

json SaveData::toJson() const {
  json j = json::object();
  // Items
  auto &j_items = j["items"];
  for (const auto &item : items) {
    j_items[item.first] = item.second.serialize();
  }
  return j;
}

void SaveData::fromJson(const json &j) {
  if (j.contains("items") && j["items"].is_object()) {
    const auto &j_items = j["items"];

    // Iterate through the nested "items" object
    for (auto it = j_items.begin(); it != j_items.end(); ++it) {
      std::string key = it.key();
      auto &value = it.value();

      std::string valueStr = value.is_string() ? value.get<std::string>() : "0";
      items[key] = BigNum::deserialize(valueStr);
    }
  }
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
