#include <ctime>

#include "SystemManager.hpp"
#include "./systems/ResourceManager.hpp"
#include "./systems/ScreenManager.hpp"

#include "game.hpp"

void SystemManager::init() {
    std::println(stderr, "Registering systems...");

    ResourceManager::init();
    SystemManager::instance().registerSystem(&ResourceManager::instance());

    ScreenManager::init();
    SystemManager::instance().registerSystem(&ScreenManager::instance());

}

void System::onInit() {};

void System::onTick() {};

void System::requestExit() {
    GameInternals::exit = true;
}

SystemManager &SystemManager::instance() {
    static SystemManager instance;
    return instance;
}


void SystemManager::registerSystem(System *system) {
    systems.push_back(system);
}

void SystemManager::onTick() {
    time_t start = time(nullptr);
    for (const auto &system: systems) {
        system->onTick();
    }
    time_t end = time(nullptr);

    // Calculate time to sleep
    double delta = difftime(end, start);
    double sleep_time = 1.0 / FRAME_RATE - delta;
    if (sleep_time > 0) {
        timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = sleep_time * 1e9;
        nanosleep(&ts, nullptr);
    }
}
