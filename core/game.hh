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

// Thread-safe game data class
class GameData {
private:
    BigNum points;
    std::mutex mtx_points;
    std::unordered_map<std::string, BigNum> resources;
    std::mutex mtx_resources;
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

    BigNum getResource(const std::string id) {
        std::lock_guard<std::mutex> lock(mtx_resources);
        if (!resources.contains(id)) {
            return BigNum(0);
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
};

typedef std::shared_ptr<GameData> GameDataPtr;

// Convert game data to json
json to_json(const GameDataPtr data) {
    json resources_json = json::object({});
    for (const auto &id : resourceTypes.getResourceIds()) {
        BigNum val = data->getResource(id);
        if (val != 0) {
            resources_json[id] = val.serialize();
        }
    }
    return json{
        {"points", data->getPoints().serialize()},
        {"resources", resources_json}
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