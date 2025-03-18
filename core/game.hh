#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <optional>

#include <unordered_map>
#include <unordered_set>

#include "BigNum.hh"
#include "json.hh"

using std::string;
using nlohmann::json;
using N = BigNum;

static constexpr int GAME_TICK_SPEED = 30;

class ResourceTypes {
private:
    std::unordered_set<std::string> ResourceIds;
    std::mutex mtx;
public:
    void registerResource(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx);
        ResourceIds.insert(id);
    }
    std::unordered_set<std::string> getResourceIds() {
        std::lock_guard<std::mutex> lock(mtx);
        return ResourceIds;
    }
    bool contains(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx);
        return ResourceIds.contains(id);
    }
};
extern ResourceTypes resourceTypes;

class UpgradeTypes {
private:
    std::unordered_set<std::string> UpgradeIds;
    std::mutex mtx;
public:
    void registerUpgrade(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx);
        UpgradeIds.insert(id);
    }
    std::unordered_set<std::string> getUpgradeIds() {
        std::lock_guard<std::mutex> lock(mtx);
        return UpgradeIds;
    }
    bool contains(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx);
        return UpgradeIds.contains(id);
    }
};
extern UpgradeTypes upgradeTypes;

class UnlockTypes {
private:
    std::unordered_set<std::string> UnlockIds;
    std::mutex mtx;
public:
    void registerUnlock(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx);
        UnlockIds.insert(id);
    }
    std::unordered_set<std::string> getUnlockIds() {
        std::lock_guard<std::mutex> lock(mtx);
        return UnlockIds;
    }
    bool contains(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx);
        return UnlockIds.contains(id);
    }
};
extern UnlockTypes unlockTypes;

// Thread-safe game data class
class GameData {
private:
    BigNum points;
    std::mutex mtx_points;
    std::unordered_map<std::string, BigNum> resources;
    std::mutex mtx_resources;
    std::unordered_map<std::string, BigNum> upgrades;
    std::mutex mtx_upgrades;
    std::unordered_set<std::string> unlocks;
    std::mutex mtx_unlocks;
public:
    GameData() : points(BigNum(0)) {}
    GameData(BigNum points) : points(points) {}

    BigNum getPoints() {
        std::lock_guard<std::mutex> lock(mtx_points);
        return points;
    }

    void setPoints(const BigNum& newPoints) {
        std::lock_guard<std::mutex> lock(mtx_points);
        points = newPoints;
    }

    void addPoints(const BigNum& add) {
        std::lock_guard<std::mutex> lock(mtx_points);
        points += add;
    }

    void subPoints(const BigNum& sub) {
        std::lock_guard<std::mutex> lock(mtx_points);
        points -= sub;
    }

    std::optional<BigNum> getResource(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        if (!resources.contains(id)) {
            return std::nullopt;
        }
        return resources[id];
    }

    void setResource(const std::string id, const BigNum& value) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        resources[id] = value;
    }

    void addResource(const std::string& id, const BigNum& add) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        if (!resources.contains(id)) {
            resources[id] = BigNum(0);
        }
        resources[id] += add;
    }

    std::optional<BigNum> getUpgrade(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx_upgrades);
        if (!upgrades.contains(id)) {
            return std::nullopt;
        }
        return upgrades[id];
    }

    void setUpgrade(const std::string id, const BigNum& value) {
        std::lock_guard<std::mutex> lock(mtx_upgrades);
        upgrades[id] = value;
    }

    void addUpgrade(const std::string& id, const BigNum& add) {
        std::lock_guard<std::mutex> lock(mtx_upgrades);
        if (!upgrades.contains(id)) {
            upgrades[id] = BigNum(0);
        }
        upgrades[id] += add;
    }

    bool hasUnlock(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx_unlocks);
        return unlocks.contains(id);
    }

    void addUnlock(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx_unlocks);
        unlocks.insert(id);
    }
};

typedef std::shared_ptr<GameData> GameDataPtr;

// Convert game data to json
json to_json(const GameDataPtr data) {
    json resources_json = json::object({});
    for (const auto &id : resourceTypes.getResourceIds()) {
        auto val = data->getResource(id);
        if (val.has_value()) {
            resources_json[id] = val.value().serialize();
        }
    }
    json upgrades_json = json::object({});
    for (const auto &id : upgradeTypes.getUpgradeIds()) {
        auto val = data->getUpgrade(id);
        if (val.has_value()) {
            upgrades_json[id] = val.value().serialize();
        }
    }
    json unlocks_json = json::array({});
    for (const auto &id : unlockTypes.getUnlockIds()) {
        unlocks_json.push_back(id);
    }
    return json{
        {"points", data->getPoints().serialize()},
        {"resources", resources_json},
        {"upgrades", upgrades_json},
        {"unlocks", unlocks_json}
    };
}

// Convert json to game data
std::shared_ptr<GameData> from_json(const json& j) {
    std::shared_ptr<GameData> data(new GameData());
    try {
        // Load points
        string pts_str;
        j.at("points").get_to(pts_str);
        data->setPoints(BigNum(pts_str));

        // Load resources
        for (const auto& [id, val] : j.at("resources").items()) {
            string val_str;
            if (!val.is_string() || !resourceTypes.contains(id)) {
                throw std::runtime_error("Resource id is not a valid type: '" + id + "'");
            }
            val.get_to(val_str);
            data->setResource(id, BigNum(val_str));
        }

        // Load upgrades
        for (const auto& [id, val] : j.at("upgrades").items()) {
            string val_str;
            if (!val.is_string() || !upgradeTypes.contains(id)) {
                throw std::runtime_error("Upgrade id is not a valid type: '" + id + "'");
            }
            val.get_to(val_str);
            data->setUpgrade(id, BigNum(val_str));
        }

        // Load unlocks
        for (const auto& val : j.at("unlocks")) {
            if (!val.is_string() || !unlockTypes.contains(val)) {
                throw std::runtime_error("Unlock id is not a valid type: '" + val.get<string>() + "'");
            }
            data->addUnlock(val.get<string>());
        }

        return data;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

// Save game data
void save(GameDataPtr data, const string& filename) {
    std::cout << "Saving game data to " << filename << "..." << std::endl;
    
    // Convert to json
    json j = to_json(data);

    std::ofstream o(filename);
    if (!o.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
        return;
    }
    o << j.dump(0) << std::endl;

    std::cout << "Game data saved!" << std::endl;
}

// Load game data
GameDataPtr load(const string& filename) {
    std::cout << "Loading game data from " << filename << "..." << std::endl;

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "File not found, creating new game data..." << std::endl;
        return std::shared_ptr<GameData>(new GameData());
    }

    json j;
    try {
        file >> j;
    } catch(const std::exception& e) {
        std::cout << "Error: Could not parse json! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not parse json");
    }
    auto data = from_json(j);
    if (!data) {
        std::cout << "Error: Could not load game data! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not load game data");
    }

    std::cout << "Game data loaded!" << std::endl;
    return data;
}