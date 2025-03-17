#pragma once

#include <iostream>
#include "../core/game.hh"
#include "./Clicker.hh"

class Factory {
private:
    inline static const uint TICK_INTERVAL = 30;

    Factory() {
        // Register resource ids
        std::cerr << "Registering resource ids" << std::endl;
        resourceTypes.registerResource(factory);
    }

public:
    inline static const std::string factory = "factory";

    static Factory& getInstance() {
        static Factory instance;
        return instance;
    }

    static void create() {
        (void) getInstance();
    }

    inline static BigNum getLevel(const GameDataPtr data) {
        return data->getResource(factory);
    }

    // Clickers per second
    inline static BigNum getCpS(const GameDataPtr data) {
        return getLevel(data);
    }

    // Cost to buy next Factory
    inline static BigNum getCost(const GameDataPtr data) {
        return getLevel(data).pow(2) * 10;
    }

    static void onTick(const GameDataPtr data, const uint gameTick) {
        // Process factories
        if (gameTick % TICK_INTERVAL == 0) {
            data->addResource(Clicker::clicker, getCpS(data));
        }
    }

};