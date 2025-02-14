#pragma once
#include <iostream>
#include <fstream>
#include <string>

#include "BigNum.hh"
#include "json.hh"

using std::string;
using nlohmann::json;

typedef struct {
    BigNum points;
} GAME_DATA;

const GAME_DATA DEFAULT_GAME_DATA = {
    points: BigNum(0)
};
typedef std::shared_ptr<GAME_DATA> GameDataPtr;

// Convert game data to json
json to_json(const GameDataPtr data) {
    return json{
        {"points", data->points.to_string()}
    };
}

// Convert json to game data
bool from_json(const json& j, GAME_DATA& data) {
    try {
        string pts_str;
        j.at("points").get_to(pts_str);
        data.points = BigNum(pts_str);
        return true;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    
}

// Save game data
void save(GameDataPtr data, string filename) {
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
GameDataPtr load(string filename) {
    std::cout << "Loading game data from " << filename << "..." << std::endl;
    GAME_DATA data;

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "File not found, creating new game data..." << std::endl;
        data = DEFAULT_GAME_DATA;
        return std::make_shared<GAME_DATA>(data);
    }

    json j;
    try {
        file >> j;
    } catch(const std::exception& e) {
        std::cout << "Error: Could not parse json! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not parse json");
    }

    bool success = from_json(j, data);
    if (!success) {
        std::cout << "Error: Could not load game data! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not load game data");
    }

    std::cout << "Game data loaded!" << std::endl;
    return std::make_shared<GAME_DATA>(data);
}