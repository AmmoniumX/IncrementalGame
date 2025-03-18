#pragma once

#include <iostream>
#include "../core/game.hh"

class Clicker {
private:
    Clicker() {
        // Register resource ids
        std::cerr << "Registering resource ids" << std::endl;
        resourceTypes.registerResource(clicker);
        upgradeTypes.registerUpgrade(clicker_lvl);
    }

public:
    inline static constexpr std::string clicker = "clicker";
    inline static constexpr std::string clicker_lvl = "clicker_lvl";

    static Clicker& getInstance() {
        static Clicker instance;
        return instance;
    }

    static void create() {
        (void) getInstance();
    }

    inline static BigNum getCount(const GameDataPtr data) {
        return data->getResource(clicker).value_or(N(0));
    }

    inline static BigNum getLevel(const GameDataPtr data) {
        return data->getUpgrade(clicker_lvl).value_or(N(1));
    }

    inline static double getSpC(const GameDataPtr data) {
        const int lvl_bonus = data->getUpgrade(clicker_lvl).value_or(N(1)).to_number().value_or(1) - 1;
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*lvl_bonus), 1);
        const double clicker_freq_secs = static_cast<double>(clicker_freq_ticks) / static_cast<double>(GAME_TICK_SPEED);
        return clicker_freq_secs;
    }
    
    inline static BigNum getCps(const GameDataPtr data) {
        const double clicker_freq_secs = getSpC(data);
        BigNum clickers = data->getResource(clicker).value_or(N(0));
        return clickers * clicker_freq_secs;
    }

    inline static BigNum getCost(const GameDataPtr data) {
        return (data->getResource(clicker).value_or(N(0))+1).pow(1.15f) * 10;
    }

    static void onTick(const GameDataPtr data, const uint gameTick) {
        // Process clickers
        const int lvl_bonus = data->getUpgrade(clicker_lvl).value_or(N(1)).to_number().value_or(0);
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*lvl_bonus), 1);
        BigNum clickers = data->getResource(clicker).value_or(N(0));
        if (gameTick % clicker_freq_ticks == 0) { data->addPoints(clickers); }
    }

    // Delete copy constructor and assignment operator to prevent copying
    Clicker(const Clicker&) = delete;
    Clicker& operator=(const Clicker&) = delete;
};