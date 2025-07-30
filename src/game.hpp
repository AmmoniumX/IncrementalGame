#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "BigNum.hpp"
#include "json.hpp"
#include "ResourceManager.hpp"

using std::string;
using nlohmann::json;
using N = BigNum;

static constexpr int GAME_TICK_SPEED = 30;

// helper json method
template <typename T>
T get_or(const json& j, const std::string& key, const T& default_value) {
    if (j.contains(key)) {
        return j[key].get<T>();
    }
    return default_value;
}

// Convert game data to json
json to_json() {
    return ResourceManager.serialize();
}

// Convert json to game data
void from_json(const json& j) {
    ResourceManager.deserialize(j);
}

// Save game data
void save(const string& filename) {
    std::println(std::cerr, "Saving game data to {}", filename);
    
    // Convert to json
    json j = to_json();

    std::ofstream o(filename);
    if (!o.is_open()) {
        std::println(std::cerr, "Error: Could not open file {}", filename);
        return;
    }
    o << j.dump(0) << std::endl;

    std::println(std::cerr, "Game data saved!");
}

// Load game data
void load(const string& filename) {
    std::println(std::cerr, "Loading game data from {}", filename);

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println(std::cerr, "File not found, ResourceManager will be empty!");
        return;
    }

    json j;
    try {
        file >> j;
    } catch(const std::exception& e) {
        std::println(std::cerr, "Error: Could not parse json! Is data corrupted? {}", e.what());
        throw std::runtime_error("Could not parse json");
    }
    from_json(j);

    std::println(std::cerr, "Game data loaded!");
    return;
}
