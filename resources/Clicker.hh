#pragma once

#include <iostream>
#include "../core/game.hh"

class Clicker {
private:
    Clicker() {
        // Register resource ids
        std::cerr << "Registering resource ids" << std::endl;
        resourceTypes.registerResource(clicker);
        resourceTypes.registerResource(clicker_lvl);
    }

public:
    inline static const std::string clicker = "clicker";
    inline static const std::string clicker_lvl = "clicker_lvl";

    static Clicker& getInstance() {
        static Clicker instance;
        return instance;
    }

    static void create() {
        (void) getInstance();
    }

    static void onTick(const GameDataPtr data, const uint gameTick) {
        // Process clickers
        int clicker_freq = std::max(static_cast<int>(GAME_TICK_SPEED - 3*data->getResource(clicker_lvl).to_number().value_or(10)), 1);
        BigNum clickers = data->getResource(clicker);
        if (gameTick % clicker_freq == 0) { data->addPoints(clickers); }
    }

    // Delete copy constructor and assignment operator to prevent copying
    Clicker(const Clicker&) = delete;
    Clicker& operator=(const Clicker&) = delete;
};