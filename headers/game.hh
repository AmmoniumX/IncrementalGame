#pragma once
#include <boost/multiprecision/gmp.hpp>
#include "json.hh"
#include <iostream>
#include <fstream>
#include <string>

typedef boost::multiprecision::mpz_int bigint;
using std::string;
using nlohmann::json;

typedef struct {
    bigint points;
} GAME_DATA;

GAME_DATA DEFAULT_GAME_DATA = {
    points: bigint(0)
};

// Convert game data to json
json to_json(const GAME_DATA& data) {
    return json{
        {"points", data.points.str()}
    };
}

// Convert json to game data
bool from_json(const json& j, GAME_DATA& data) {
    try {
        string pps_str;
        j.at("points").get_to(pps_str);
        data.points = bigint(pps_str);
        return true;
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    
}

// Save game data
void save(GAME_DATA& data, string filename) {
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
GAME_DATA load(string filename) {
    std::cout << "Loading game data from " << filename << "..." << std::endl;

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "File not found, creating new game data..." << std::endl;
        return DEFAULT_GAME_DATA;
    }

    json j;
    try {
        file >> j;
    } catch(const std::exception& e) {
        std::cout << "Error: Could not parse json! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not parse json");
    }

    GAME_DATA data;
    bool success = from_json(j, data);
    if (!success) {
        std::cout << "Error: Could not load game data! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not load game data");
    }

    std::cout << "Game data loaded!" << std::endl;
    return data;
}