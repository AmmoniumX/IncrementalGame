#pragma once

#include <mutex>
#include <print>
#include <vector>

#include "setup.hpp"

class System {
public:
    virtual ~System() = default;
    virtual std::string_view getId() const = 0;
    virtual void onTick() {}

    void requestExit() {
        GameInternals::exit = true;
    }
};

class SystemManager {
private:
    SystemManager() = default;
    std::vector<System*> systems;
    std::mutex mtx;

public:
    static SystemManager &instance() {
        static SystemManager instance;
        return instance;
    }

    void registerSystem(System *system) {
        std::lock_guard lock(mtx);
        systems.push_back(system);
    }

    void onTick() {
        for (const auto &system: systems) {
            system->onTick();
        }
    }
};

// CRTP helper for automatic registration
template <typename Derived>
class RegisteredSystem : public System {
public:
    std::string_view getId() const override {
        return Derived::RESOURCE_ID;
    }

    RegisteredSystem() = default;

    static void registerSystem() {
        static bool registered = false;
        if (!registered) {
            registered = true;
            SystemManager::instance().registerSystem(&Derived::instance());
        }
    }
};
