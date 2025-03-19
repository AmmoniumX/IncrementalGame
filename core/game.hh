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
#include "resourceRegistry.hh"

using std::string;
using nlohmann::json;
using N = BigNum;

static constexpr int GAME_TICK_SPEED = 30;

// Convert game data to json
json to_json() {
    return ResourceRegistry.serialize();
}

// Convert json to game data
void from_json(const json& j) {
    ResourceRegistry.deserialize(j);
}

// Save game data
void save(const string& filename) {
    std::cerr << "Saving game data to " << filename << "..." << std::endl;
    
    // Convert to json
    json j = to_json();

    std::ofstream o(filename);
    if (!o.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    o << j.dump(0) << std::endl;

    std::cerr << "Game data saved!" << std::endl;
}

// Load game data
void load(const string& filename) {
    std::cerr << "Loading game data from " << filename << "..." << std::endl;

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "File not found, ResourceRegistry will be empty..." << std::endl;
        return;
    }

    json j;
    try {
        file >> j;
    } catch(const std::exception& e) {
        std::cerr << "Error: Could not parse json! Is data corrupted?" << std::endl;
        throw std::runtime_error("Could not parse json");
    }
    from_json(j);

    std::cerr << "Game data loaded!" << std::endl;
    return;
}