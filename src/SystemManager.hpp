#pragma once

#include <mutex>
#include <print>
#include <vector>

#include "setup.hpp"

class System {
public:
    virtual ~System() = default;
    virtual void onInit() {}
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

    static void init();

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

