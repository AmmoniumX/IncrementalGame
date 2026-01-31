#include <thread>

#include "./systems/ScreenManager.hpp"
#include "Logger.hpp"
#include "SystemManager.hpp"
#include "game.hpp"

void SystemManager::init() {
  Logger::println("Registering systems...");

  ScreenManager::init();
  SystemManager::instance().registerSystem(&ScreenManager::instance());
}

void System::onInit() {};

void System::onTick() {};

void System::requestExit() { Game::exit = true; }

SystemManager &SystemManager::instance() {
  static SystemManager instance;
  return instance;
}

void SystemManager::registerSystem(System *system) {
  systems.push_back(system);
}

void SystemManager::onTick() {
  auto start = Clock::now();
  for (const auto &system : systems) {
    system->onTick();
  }
  auto end = Clock::now();

  // Calculate time to sleep
  Duration delta = end - start;
  Duration sleep_time = TARGET_TICK_TIME - delta;

  if (sleep_time > 0ms) {
    // Logger::println("Trying to sleep for {}ms...",
    // std::chrono::duration_cast<std::chrono::milliseconds>(sleep_time));
    std::this_thread::sleep_for(sleep_time);
  }
}
