#include <atomic>
#include <ctime>
#include <getopt.h>
#include <iostream>
#include <ncursesw/ncurses.h>
#include <print>
#include <thread>
#include <string>

#include "./render/Screen.hpp"
#include "./SystemManager.hpp"
#include "./systems/ScreenManager.hpp"
#include "./systems/ResourceManager.hpp"
#include "./screens/MainScreen.hpp"
#include "./resources/Inventory.hpp"
#include "./resources/Recipes.hpp"
#include "game.hpp"
#include "setup.hpp"

void ResourceManager::init() {
    std::println(stderr, "Registering resources...");
    Inventory::init();
    Recipes::init();
}

void ScreenManager::init() {
    std::println(stderr, "Registering screens...");
    // Create and setup ScreenManager and Screen
    ScreenManager &instance = ScreenManager::instance();
    std::unique_ptr<Screen> mainScreen = MainScreen::create();
    std::reference_wrapper<Screen> movedMainScreen = instance.registerScreen(std::move(mainScreen));
    instance.changeScreen(&movedMainScreen.get());
}

void SystemManager::init() {
    std::println(stderr, "Registering systems...");

    ResourceManager::init();
    SystemManager::instance().registerSystem(&ResourceManager::instance());

    ScreenManager::init();
    SystemManager::instance().registerSystem(&ScreenManager::instance());

}

using nlohmann::json;

void gameTick() {
    SystemManager::instance().onTick();
}

void run() {
    std::println(stderr, "Running game...");
    // Main game loop
    do {
        auto start = time(nullptr);
        gameTick();
        auto end = time(nullptr);
        double delta = difftime(end, start);
        double sleep_time = 1.0 / GAME_TICK_SPEED - delta;
        if (sleep_time > 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>(sleep_time * 1000)));
        }
    } while (!GameInternals::exit);
    std::println(stderr, "Exiting...");
}

void init(std::string savefile) {
    
    // Initialize ncurses
    std::println(stderr, "Initializing ncurses...");
    setupNcurses();    

    // Initialize systems
    SystemManager::init();

    // Load game data
    load(savefile);
}

void cleanup(std::string savefile) {
    // Save game data
    save(savefile);
}

int main(int argc, char *argv[]) {
    string savefile = "saves/save.json";

    // Parse arguments
    int opt;
    static struct option long_options[] = {{"save", required_argument, 0, 0},
                                           {0, 0, 0, 0}};
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "", long_options, &option_index)) !=
           -1) {
        if (opt == 0 && strcmp(long_options[option_index].name, "save") == 0) {
            savefile = optarg;
        } else {
            std::println("Usage: {} [--save savefile]", argv[0]);
            return 1;
        }
    }

    // Setup
    init(savefile);
    run();
    cleanup(savefile);

    return 0;
}
