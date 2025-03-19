#pragma once

#include <iostream>
#include "../core/game.hh"
#include "../core/resourceRegistry.hh"

class Clicker: public RegisteredResource<Clicker> {
private:
    BigNum count = N(0);
    BigNum level = N(1);

public:
    static constexpr const char* RESOURCE_ID = "clicker";
    Clicker() {
        std::cerr << "Instantiating Clicker" << std::endl;
    }
    
    static std::shared_ptr<Clicker> getInstance() {
        static std::shared_ptr<Clicker> instance = std::make_shared<Clicker>();
        registerResource();
        return instance;
    }

    virtual json serialize() const override {
        json j;
        j["count"] = count.to_string();
        j["level"] = level.to_string();
        return j;
    }

    virtual void deserialize(const json& j) override {
        if (j.contains("count")) { count = BigNum(j["count"].get<string>()); }
        if (j.contains("level")) { level = BigNum(j["level"].get<string>()); }
    }

    BigNum getCount() {
        return count;
    }

    BigNum getLevel() {
        return level;
    }

    void addCount(const BigNum& amount) {
        count += amount;
    }

    void addLevel(const BigNum& amount) {
        level += amount;
    }

    double getSpC() {
        const int lvl_bonus = level.to_number().value_or(1) - 1;
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*lvl_bonus), 1);
        const double clicker_freq_secs = static_cast<double>(clicker_freq_ticks) / static_cast<double>(GAME_TICK_SPEED);
        return clicker_freq_secs;
    }
    
    BigNum getCps() {
        const double clicker_freq_secs = getSpC();
        return count * clicker_freq_secs;
    }

    BigNum getCost() {
        return ((count+1)*10).pow(1.15f);
    }

    virtual void onTick(const uint &gameTick) override {
        // Process clickers
        if (count == 0) { return; }
        const int lvl_bonus = level.to_number().value_or(1) - 1;
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*lvl_bonus), 1);
        if (gameTick % clicker_freq_ticks == 0) { 
            ResourceRegistry.addPoints(count); 
        }
    }

    // Delete copy constructor and assignment operator to prevent copying
    Clicker(const Clicker&) = delete;
    Clicker& operator=(const Clicker&) = delete;
};