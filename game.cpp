#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>

#include "headers/game.hh"
#include "headers/json.hh"
#include "headers/render.hh"

using std::cin, std::cout, std::cerr, std::endl;
using std::string;
using nlohmann::json;

// Convert game data to json
json to_json(const GAME_DATA& data) {
    return json{
        {"points", data.points.str()}
    };
}

// Convert json to game data
bool from_json(const json& j, GAME_DATA& data) {
    try {
        string pps_str;
        j.at("points").get_to(pps_str);
        data.points = bigint(pps_str);
        return true;
    } catch(const std::exception& e) {
        cerr << e.what() << endl;
        return false;
    }
    
}

// Save game data
void save(GAME_DATA& data, string filename) {
    cout << "Saving game data to " << filename << "..." << endl;
    
    // Convert to json
    json j = to_json(data);

    std::ofstream o(filename);
    if (!o.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }
    o << j.dump(0) << std::endl;

    cout << "Game data saved!" << endl;
}

// Load game data
GAME_DATA load(string filename) {
    cout << "Loading game data from " << filename << "..." << endl;

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        cout << "File not found, creating new game data..." << endl;
        return DEFAULT_GAME_DATA;
    }

    json j;
    try {
        file >> j;
    } catch(const std::exception& e) {
        cout << "Error: Could not parse json! Is data corrupted?" << endl;
        throw std::runtime_error("Could not parse json");
    }

    GAME_DATA data;
    bool success = from_json(j, data);
    if (!success) {
        cout << "Error: Could not load game data! Is data corrupted?" << endl;
        throw std::runtime_error("Could not load game data");
    }

    cout << "Game data loaded!" << endl;
    return data;
}

int run(string savefile) {
    // Load game data
    GAME_DATA data = load(savefile);

    // Create and setup ScreenManager and Screen
    ScreenManager &manager = ScreenManager::getInstance();
    ScreenPtr mainScreen = std::make_shared<Screen>();
    TextPtr mainScreenTitle = mainScreen->putText(0, 0, "Hello, world!");
    std::string points = "Points: " + data.points.str();
    TextPtr mainScreenScore = mainScreen->putText(1, 0, points);
    TextPtr mainScreenInfo = mainScreen->putText(2, 0, "Press 'ENTER' to earn points, 'q' to quit");

    mainScreen->setOnTick([&](GAME_DATA *data, const char input) {
        mainScreenScore->setText("Points: " + data->points.str());
        switch (input) {
            case 'q':
                return true;
            case '\n':
                data->points += 1;
                return false;
            case -1:
                return false;
            default:
                mvprintw(LINES-1, 0, "Unknown command: %c (%d)", input, (int)input);
                return false;
        }
    });
    manager.initialize(&data, mainScreen);
    
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
