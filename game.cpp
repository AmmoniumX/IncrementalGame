#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>

#include "headers/game.hpp"
#include "headers/json.hpp"

using std::cin, std::cout, std::endl;
using std::string;
using nlohmann::json;

// Convert game data to json
json to_json(const GAME_DATA& data) {
    return json{
        {"pps", data.pps.str()}
    };
}

// Convert json to game data
bool from_json(const json& j, GAME_DATA& data) {
    try
    {
        string pps_str;
        j.at("pps").get_to(pps_str);
        data.pps = bigint(pps_str);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
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
    o << j.dump(4) << std::endl;
    o.close();

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
    json j = json::parse(file);
    file.close();

    GAME_DATA data;
    bool success = from_json(j, data);
    if (!success) {
        cout << "Error: Could not load game data! Is data corrupted?" << endl;
        throw std::runtime_error("Could not load game data");
    }

    cout << "Game data loaded!" << endl;
    return data;
}

// Main game loop
bool update(GAME_DATA& data) {
    clear();
    printw("Hello, world!");
    printw("\nPoints: %s", data.pps.str().c_str());
    printw("\nENTER to gain points, q to quit");
    
    char c = getch();
    switch (c)
    {
    case 'q':
        return false;
    case '\n':
        data.pps += 1;
        break;
    default:
        cout << "Unknown command: " << c << endl;
        break;
    }
    refresh();
    return true;
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

    // Load game data
    GAME_DATA data = load(savefile);

    // Main game loop
    initscr();
    while (update(data)) { }
    endwin();

    // Save game data
    save(data, savefile);

    return 0;
}
