#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "BigNum.hpp"
#include "ResourceManager.hpp"
#include "json.hpp"

using nlohmann::json;
using std::string;
using N = BigNum;

static constexpr int GAME_TICK_SPEED = 30;

// helper json method
template <typename T>
T get_or(const json &j, const std::string &key, const T &default_value) {
    if (j.contains(key)) {
        return j[key].get<T>();
    }
    return default_value;
}

// Convert game data to json
json to_json() { return ResourceManager::instance().serialize(); }

// Convert json to game data
void from_json(const json &j) { ResourceManager::instance().deserialize(j); }

// Save game data
void save(const string &filename) {
    std::println(stderr, "Saving game data to {}", filename);

    // Convert to json
    json j = to_json();

    std::ofstream o(filename);
    if (!o.is_open()) {
        std::println(stderr, "Error: Could not open file {}", filename);
        return;
    }
    o << j.dump(0) << std::endl;

    std::println(stderr, "Game data saved!");
}

// Load game data
void load(const string &filename) {
    std::println(stderr, "Loading game data from {}", filename);

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println(stderr,
                     "File not found, ResourceManager will be empty!");
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception &e) {
        std::println(stderr,
                     "Error: Could not parse json! Is data corrupted? {}",
                     e.what());
        throw std::runtime_error("Could not parse json");
    }
    from_json(j);

    std::println(stderr, "Game data loaded!");
    return;
}
