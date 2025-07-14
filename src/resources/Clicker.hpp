#pragma once

#include <iostream>
#include "../game.hpp"
#include "../resourceRegistry.hpp"
#include "./Points.hpp"

class Clicker: public RegisteredResource<Clicker> {
private:
    BigNum count = N(0);
    BigNum speed = N(1);
    BigNum prod = N(1);
    std::shared_ptr<Points> points;

public:
    static constexpr const char* RESOURCE_ID = "clicker";
    static const BigNum MAX_SPEED;

    Clicker() {
        std::cerr << "Instantiating Clicker" << std::endl;
        points = ResourceRegistry.getResource<Points>(Points::RESOURCE_ID);
    }
    
    static std::shared_ptr<Clicker> getInstance() {
        static std::shared_ptr<Clicker> instance = std::make_shared<Clicker>();
        registerResource();
        return instance;
    }

    virtual json serialize() const override {
        json j;
        j["count"] = count.to_string();
        j["speed"] = speed.to_string();
        j["prod"] = prod.to_string();
        return j;
    }

    virtual void deserialize(const json& j) override {
        count = BigNum(get_or<string>(j, "count", "0"));
        speed = BigNum(get_or<string>(j, "speed", "1"));
        prod = BigNum(get_or<string>(j, "prod", "1"));
    }

    BigNum getCount() {
        return count;
    }

    BigNum getSpeed() {
        return speed;
    }

    BigNum getProd() {
        return prod;
    }

    void addCount(const BigNum& amount) {
        count += amount;
    }

    void addSpeed(const BigNum& amount) {
        speed = std::min(speed + amount, N(MAX_SPEED));
    }

    void addProd(const BigNum& amount) {
        prod += amount;
    }

    double getSpC() {
        const int speed_bonus = speed.to_number().value_or(1) - 1;
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*speed_bonus), 1);
        const double clicker_freq_secs = static_cast<double>(clicker_freq_ticks) / static_cast<double>(GAME_TICK_SPEED);
        return clicker_freq_secs;
    }
    
    BigNum getCps() {
        const double clicker_freq_secs = getSpC();
        return count * clicker_freq_secs;
    }

    BigNum getCost() {
        return (count+1).pow(1.15f) * 10;
    }

    BigNum getSpeedCost() {
        return speed.pow(1.5f) * 100;
    }

    BigNum getProdCost() {
        return prod.pow(2.0f) * 1000;
    }

    virtual void onTick(const uint &gameTick) override {
        // Process clickers
        if (count == 0) { return; }
        const int speed_bonus = speed.to_number().value_or(1) - 1;
        const int clicker_freq_ticks = std::max(static_cast<int>(GAME_TICK_SPEED - 3*speed_bonus), 1);
        if (gameTick % clicker_freq_ticks == 0) { 
            points->addPoints(count * prod);
        }
    }

    // Delete copy constructor and assignment operator to prevent copying
    Clicker(const Clicker&) = delete;
    Clicker& operator=(const Clicker&) = delete;
};

const BigNum Clicker::MAX_SPEED = N(10);
