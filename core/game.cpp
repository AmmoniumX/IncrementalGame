#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>
#include <thread>
#include <ctime>
#include <atomic>

#include "game.hh"
#include "render.hh"
#include "../screens/MainScreen.hh"
#include "../resources/Clicker.hh"
#include "../resources/Factory.hh"

using std::cout, std::endl;
using nlohmann::json;

// Define extern variables
ResourceTypes resourceTypes;
UpgradeTypes upgradeTypes;
UnlockTypes unlockTypes;

namespace {
    // Private game variables
    uint tick = 0;
    std::atomic_bool do_exit = false;
    GameDataPtr data = nullptr;
}

void gameTick() {
    if (!data) { std::cerr << "GameData is null" << std::endl; return; }
    Clicker::onTick(data, tick);
    Factory::onTick(data, tick);
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
    std::cerr << "Creating resources" << std::endl;
    Clicker::create();
    Factory::create();

    // Load game data
    std::cerr << "Loading game data" << std::endl;
    data = load(savefile);

    // Initialize ncurses
    std::cerr << "Setting up ncurses" << std::endl;
    setupNcurses();

    // Create and setup ScreenManager and Screen
    std::cerr << "Creating mainScreen" << std::endl;
    ScreenPtr mainScreen = MainScreen::create(data);
    std::cerr << "Creating ScreenManager" << std::endl;
    ScreenManager &manager = ScreenManager::getInstance(data, mainScreen);

    // Main game loop
    std::cerr << "Starting game thread" << std::endl;
    do_exit = false;
    std::thread gameThread([]() {
        gameWorker(); return;
    });
    std::cerr << "Starting render loop" << std::endl;
    manager.run();

    // Cleanup
    do_exit = true;
    gameThread.join();
    save(data, savefile);

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
            cout << "Usage: " << argv[0] << " [--save savefile]" << endl;
            return 1;
        }
    }

    return run(savefile);
}
