#pragma once

#include <iostream>
#include "../game.hpp"
#include "../resourceRegistry.hpp"
#include "./Points.hpp"
#include "./Clicker.hpp"

class Factory : public RegisteredResource<Factory> {
private:
    std::shared_ptr<Clicker> clicker;
    const uint TICK_INTERVAL = 30;
    BigNum count = N(0);

public:
    static constexpr const char* RESOURCE_ID = "factory";
    Factory() {
        std::cerr << "Instantiating Factory" << std::endl;
        clicker = ResourceRegistry.getResource<Clicker>(Clicker::RESOURCE_ID);
    }
    
    static std::shared_ptr<Factory> getInstance() {
        static std::shared_ptr<Factory> instance = std::make_shared<Factory>();
        registerResource();
        return instance;
    }
    
    virtual json serialize() const override {
        json j;
        j["count"] = count.to_string();
        return j;
    }

    virtual void deserialize(const json& j) override {
        count = BigNum(get_or<string>(j, "count", "0"));
    }
    
    BigNum getCount() {
        return count;
    }

    void addCount(const BigNum& amount) {
        count += amount;
    }

    // Clickers per second
    BigNum getCpS() {
        return getCount();
    }

    // Cost to buy next Factory
    BigNum getCost() {
        return (getCount()+1).pow(2.0f) * 10;
    }

    virtual void onTick(const uint &gameTick) override {
        // Process factories
        if (gameTick % TICK_INTERVAL == 0) {
            clicker->addCount(getCount());
        }
    }

};
