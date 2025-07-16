#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>
#include <thread>
#include <ctime>
#include <atomic>
#include <print>

#include "setup.hpp"
#include "game.hpp"
#include "./render/Window.hpp"
#include "./render/Screen.hpp"
#include "./render/ScreenManager.hpp"
#include "./screens/MainScreen.hpp"
#include "./resources/Inventory.hpp"

using std::cout, std::endl;
using nlohmann::json;

namespace {
    // Private game variables
    uint tick = 0;
    std::atomic_bool do_exit = false;
}

void gameTick() {
    ResourceManager.onTick(tick);
    tick++;
}

void gameWorker() {
    // Main game loop
    while (!do_exit) {
        auto start = time(nullptr);
        gameTick();
        auto end = time(nullptr);
        double delta = difftime(end, start);
        double sleep_time = 1.0 / GAME_TICK_SPEED - delta;
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time * 1000)));
        }
    }
}



int run(string savefile) {

    // Initialize resoruces
    std::println(std::cerr, "Creating resources");
    Inventory::create();

    // Load game data
    load(savefile);

    // Initialize ncurses
    std::println(std::cerr, "Setting up ncurses");
    setupNcurses();

    // Create and setup ScreenManager and Screen
    std::println(std::cerr, "Setting up ScreenManager and MainScreen");
    ScreenPtr mainScreen = MainScreen::create();
    ScreenManager &manager = ScreenManager::getInstance();
    manager.changeScreen(mainScreen);

    // Main game loop
    std::println(std::cerr, "Starting game thread");
    do_exit = false;
    std::thread gameThread([]() {
        gameWorker(); return;
    });
    std::println(std::cerr, "Starting render loop");
    manager.run();
    std::println(std::cerr, "Render loop ended");
    // Cleanup
    do_exit = true;
    gameThread.join();
    std::println(std::cerr, "Game thread ended");
    save(savefile);

    return 0;
}

int main(int argc, char *argv[]) {
    string savefile = "saves/save.json";

    // Parse arguments
    int opt;
    static struct option long_options[] = {
        {"save", required_argument, 0, 0},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "", long_options, &option_index)) != -1) {
        if (opt == 0 && strcmp(long_options[option_index].name, "save") == 0) {
            savefile = optarg;
        } else {
            std::println("Usage: {} [--save savefile]", argv[0]);
            return 1;
        }
    }

    return run(savefile);
}
