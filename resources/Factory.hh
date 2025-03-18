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
    inline static constexpr std::string factory = "factory";

    static Factory& getInstance() {
        static Factory instance;
        return instance;
    }

    static void create() {
        (void) getInstance();
    }

    inline static BigNum getCount(const GameDataPtr data) {
        return data->getResource(factory).value_or(N(0));
    }

    // Clickers per second
    inline static BigNum getCpS(const GameDataPtr data) {
        return getCount(data);
    }

    // Cost to buy next Factory
    inline static BigNum getCost(const GameDataPtr data) {
        return getCount(data).pow(2.0f) * 10;
    }

    static void onTick(const GameDataPtr data, const uint gameTick) {
        // Process factories
        if (gameTick % TICK_INTERVAL == 0) {
            data->addResource(Clicker::clicker, getCpS(data));
        }
    }

};