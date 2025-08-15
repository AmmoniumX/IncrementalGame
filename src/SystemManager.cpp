#include <ctime>
#include <chrono>
#include <thread>

#include "Logger.hpp"
#include "game.hpp"
#include "SystemManager.hpp"
#include "./systems/ResourceManager.hpp"
#include "./systems/ScreenManager.hpp"

void SystemManager::init() {
    Logger::println("Registering systems...");

    ResourceManager::init();
    SystemManager::instance().registerSystem(&ResourceManager::instance());

    ScreenManager::init();
    SystemManager::instance().registerSystem(&ScreenManager::instance());

}

void System::onInit() {};

void System::onTick() {};

void System::requestExit() {
    Game::exit = true;
}

SystemManager &SystemManager::instance() {
    static SystemManager instance;
    return instance;
}


void SystemManager::registerSystem(System *system) {
    systems.push_back(system);
}

void SystemManager::onTick() {
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &system: systems) {
        system->onTick();
    }
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate time to sleep
    std::chrono::duration<double> delta = end - start;
    std::chrono::duration<double> target_duration(1.0 / FRAME_RATE);
    std::chrono::duration<double> sleep_time = target_duration - delta;

    if (sleep_time.count() > 0) {
        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::nanoseconds>(sleep_time));
    }
}
