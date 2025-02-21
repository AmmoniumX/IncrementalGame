#pragma once

#include <iostream>
#include "../core/game.hh"

class Clicker {
private:
    Clicker() {
        // Register resource ids
        std::cerr << "Registering resource ids" << std::endl;
        resourceTypes.registerResource(clicker);
        resourceTypes.registerResource(clicker_lvl_bonus);
    }

public:
    inline static const std::string clicker = "clicker";
    inline static const std::string clicker_lvl_bonus = "clicker_lvl_bonus";

    static Clicker& getInstance() {
        static Clicker instance;
        return instance;
    }

    static void create() {
        (void) getInstance();
    }

    inline static BigNum getLevel(const GameDataPtr data) {
        return data->getResource(clicker_lvl_bonus) + 1;
    }

    inline static double getSpC(const GameDataPtr data) {
        const int lvl_bonus = data->getResource(clicker_lvl_bonus).to_number().value_or(0);
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*lvl_bonus), 1);
        const double clicker_freq_secs = static_cast<double>(clicker_freq_ticks) / static_cast<double>(GAME_TICK_SPEED);
        return clicker_freq_secs;
    }
    
    inline static BigNum getCps(const GameDataPtr data) {
        const double clicker_freq_secs = getSpC(data);
        BigNum clickers = data->getResource(clicker);
        return clickers * clicker_freq_secs;
    }

    static void onTick(const GameDataPtr data, const uint gameTick) {
        // Process clickers
        const int lvl_bonus = data->getResource(clicker_lvl_bonus).to_number().value_or(0);
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*lvl_bonus), 1);
        BigNum clickers = data->getResource(clicker);
        if (gameTick % clicker_freq_ticks == 0) { data->addPoints(clickers); }
    }

    // Delete copy constructor and assignment operator to prevent copying
    Clicker(const Clicker&) = delete;
    Clicker& operator=(const Clicker&) = delete;
};