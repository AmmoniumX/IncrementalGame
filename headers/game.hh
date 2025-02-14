#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <optional>

#include "BigNum.hh"
#include "json.hh"

using std::string;
using nlohmann::json;

class GameData {
private:
    BigNum points;
    std::mutex mtx;
public:
    GameData() : points(BigNum(0)) {}
    GameData(BigNum points) : points(points) {}

    // Thread-safe getter
    BigNum getPoints() {
        std::lock_guard<std::mutex> lock(mtx);
        return points;
    }

    // Thread-safe setter
    void setPoints(const BigNum& newPoints) {
        std::lock_guard<std::mutex> lock(mtx);
        points = newPoints;
    }
};

typedef std::shared_ptr<GameData> GameDataPtr;

// Convert game data to json
json to_json(const GameDataPtr data) {
    return json{
        {"points", data->getPoints().to_string()}
    };
}

// Convert json to game data
std::shared_ptr<GameData> from_json(const json& j) {
    std::shared_ptr<GameData> data(new GameData());
    try {
        string pts_str;
        j.at("points").get_to(pts_str);
        data->setPoints(BigNum(pts_str));
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