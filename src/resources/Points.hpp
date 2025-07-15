#pragma once

#include "../game.hpp"
#include "../resourceRegistry.hpp"

class Points: public RegisteredResource<Points> {
private:
    BigNum points = N(0);
public:
    static constexpr const char* RESOURCE_ID = "points";

    Points() {};

    static void getInstance() {
        static std::unique_ptr<Points> instance = std::make_unique<Points>();
        registerResource(std::move(instance));
    }

    virtual json serialize() const override {
        json j;
        j["points"] = points.to_string();
        return j;
    }

    virtual void deserialize(const json& j) override {
        points = BigNum(get_or<string>(j, "points", "0"));
    }

    virtual void onTick([[maybe_unused]] const uint &gameTick) override {
        // No specific logic for Points on tick
    }

    BigNum getPoints() const {
        return points;
    }

    void addPoints(const BigNum& amount) {
        points += amount;
    }

    void subtractPoints(const BigNum& amount) {
        points -= amount;
    }

    void setPoints(const BigNum& amount) {
        points = amount;
    }

    // Prevent copying and assignment
    Points(const Points&) = delete;
    Points& operator=(const Points&) = delete;

};