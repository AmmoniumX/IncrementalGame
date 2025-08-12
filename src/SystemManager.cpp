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
    std::lock_guard lock(mtx);
    systems.push_back(system);
}

void SystemManager::onTick() {
    for (const auto &system: systems) {
        system->onTick();
    }
}
