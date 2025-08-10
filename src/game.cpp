#include <atomic>
#include <ctime>
#include <getopt.h>
#include <iostream>
#include <ncursesw/ncurses.h>
#include <print>
#include <thread>

#include "./ResourceManager.hpp"
#include "./render/Screen.hpp"
#include "./render/ScreenManager.hpp"
#include "./resources/Inventory.hpp"
#include "./screens/MainScreen.hpp"
#include "game.hpp"
#include "setup.hpp"

using nlohmann::json;

namespace {
// Private game variables
uint tick = 0;
std::atomic_bool do_exit = false;
} // namespace

void gameTick() {
    ResourceManager::instance().onTick(tick);
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
            std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<int>(sleep_time * 1000)));
        }
    }
}

int run(string savefile) {

    // Initialize resoruces
    std::println(std::cerr, "Registering resources...");
    Inventory::create();

    // Load game data
    load(savefile);

    // Initialize ncurses
    std::println(std::cerr, "Initializing ncurses...");
    setupNcurses();

    // Create and setup ScreenManager and Screen
    ScreenManager &manager = ScreenManager::getInstance();
    std::unique_ptr<Screen> mainScreen = MainScreen::create();
    std::reference_wrapper<Screen> movedMainScreen = manager.registerScreen(std::move(mainScreen));
    manager.changeScreen(&movedMainScreen.get());

    // Main game loop
    std::println(std::cerr, "Starting game thread...");
    do_exit = false;
    std::thread gameThread([]() {
        gameWorker();
        return;
    });
    std::println(std::cerr, "Game thread started. Starting render loop...");
    manager.run();
    std::println(std::cerr, "Render loop ended. Waiting for game thread...");
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

    return run(savefile);
}
