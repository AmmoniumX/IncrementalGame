#include "Inventory.hpp"

void Inventory::init() {
    static bool registered = false;
    if (!registered) {
        ResourceManager::instance().create(RESOURCE_ID, std::shared_ptr<Inventory>(new Inventory));
        registered = true;
    }
}

const std::map<std::string, BigNum> &Inventory::getItems() const {
    return items; 
}


BigNum Inventory::getItem(const std::string_view _item) const {
    std::string item(_item);
    auto it = items.find(std::string(item));
    if (it != items.end()) {
        return it->second;
    }
    return BigNum(0); // Return 0 if item not found
}

void Inventory::setItem(const std::string_view _item, const BigNum &amount) {
    std::string item(_item);
    items[item] = amount;
}

void Inventory::addItem(const std::string_view _item, const BigNum &amount) {
    std::string item(_item);
    items[item] += amount;
}

void Inventory::subtractItem(const std::string_view _item, const BigNum &amount) {
    std::string item(_item);
    items[item] -= amount;
    if (items[item] < BigNum(0)) {
        items[item] = BigNum(0); // Prevent negative amounts
    }
}

std::optional<json> Inventory::serialize() const {
    json j = json::object();
    for (const auto &item : items) {
        j[item.first] = item.second.serialize();
    }
    return j;
}

void Inventory::deserialize(const json &j) {
    for (const auto &[key, value] : j.items()) {
        std::string valueStr =
            value.is_string() ? value.get<std::string>() : "0";
        items[key] = BigNum::deserialize(valueStr);
    }
}
