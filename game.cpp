#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>

#include "headers/game.hh"
#include "headers/render.hh"
#include "screens/MainScreen.hh"

using std::cout, std::endl;
using nlohmann::json;

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
    std::cerr << "Starting game loop" << std::endl;
    manager.run();

    // Cleanup
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
