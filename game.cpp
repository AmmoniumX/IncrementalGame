#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>

#include "headers/game.hh"
#include "headers/json.hh"
#include "headers/render.hh"
#include "headers/BigNum.hh"

using std::cout, std::endl;
using nlohmann::json;

int run(string savefile) {
    // Load game data
    GAME_DATA data = load(savefile);

    // Create and setup ScreenManager and Screen
    ScreenPtr mainScreen = std::make_shared<Screen>();
    TextPtr mainScreenTitle = mainScreen->putText(0, 0, "Hello, world!");
    std::string points = "Points: " + data.points.to_string();
    TextPtr mainScreenScore = mainScreen->putText(1, 0, points);
    TextPtr mainScreenInfo = mainScreen->putText(2, 0, "Press 'ENTER' to earn points, 'q' to quit");

    mainScreen->setOnTick([&](GAME_DATA *dataPtr, const char input) {
        mainScreenScore->setText("Points: " + dataPtr->points.to_string());
        switch (input) {
            case 'q':
                return true;
            case '\n':
                dataPtr->points++;
                return false;
            case -1:
                return false;
            default:
                mvprintw(LINES-1, 0, "Unknown command: %c (%d)", input, (int)input);
                return false;
        }
    });
    ScreenManager &manager = ScreenManager::getInstance(&data, mainScreen);
    
    // Initialize ncurses
    setupNcurses();

    // Main game loop
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
