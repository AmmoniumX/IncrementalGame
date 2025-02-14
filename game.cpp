#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>
#include <thread>
#include <ctime>
#include <atomic>

#include "headers/game.hh"
#include "headers/render.hh"
#include "screens/MainScreen.hh"

using std::cout, std::endl;
using nlohmann::json;

static const int GAME_TICK_SPEED = FRAME_RATE;
void gameTick(GameDataPtr data) {
    BigNum points = data->getPoints();
    points++;
    data->setPoints(points);
}

int run(string savefile) {
    // Load game data
    GameDataPtr data = load(savefile);

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
    std::atomic_bool exit = false;
    std::thread gameThread([&data, &exit]() {
        while (!exit) {
            auto start = time(nullptr);
            gameTick(data);
            auto end = time(nullptr);
            double delta = difftime(end, start);
            double sleep_time = 1.0 / GAME_TICK_SPEED - delta;
            if (sleep_time > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time * 1000)));
            }
        }
    });
    std::cerr << "Starting render loop" << std::endl;
    manager.run();

    // Cleanup
    exit = true;
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
