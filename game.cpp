#include <iostream>
#include <ncurses.h>
#include <string>
#include <fstream>
#include <getopt.h>
#include <ctime>

#include "headers/game.hpp"
#include "headers/json.hpp"

using std::cin, std::cout, std::cerr, std::endl;
using std::string;
using nlohmann::json;

const uint FRAME_RATE = 15;

// Convert game data to json
json to_json(const GAME_DATA& data) {
    return json{
        {"pps", data.pps.str()}
    };
}

// Convert json to game data
bool from_json(const json& j, GAME_DATA& data) {
    try {
        string pps_str;
        j.at("pps").get_to(pps_str);
        data.pps = bigint(pps_str);
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

// Main render function
void render(const GAME_DATA& data) {
    clear();
    printw("Hello, world!");
    printw("\nPoints: %d", (int) data.pps);
    printw("\nENTER to gain points, q to quit");
    refresh();
}

enum class UPDATE_STATE {
    NO_CHANGES,
    CHANGES,
    QUIT
};

UPDATE_STATE handle_input(GAME_DATA& data, char input) {
    UPDATE_STATE state = UPDATE_STATE::NO_CHANGES;
    switch (input) {
        case 'q':
            return UPDATE_STATE::QUIT;
        case '\n':
            data.pps += 1;
            state = UPDATE_STATE::CHANGES;
            break;
        default:
            printw("Unknown command: %c\n (%d)", input, (int)input);
            break;
    }
    return state;
}

bool update(GAME_DATA& data) {
    time_t start = time(nullptr);

    char input = getch();
    if (input == ERR) {
        return true;
    }

    UPDATE_STATE state = handle_input(data, input);
    
    switch (state) {
        case UPDATE_STATE::QUIT:
            return false;
        case UPDATE_STATE::CHANGES:
            render(data);
            break;
        case UPDATE_STATE::NO_CHANGES:
            break;
    }

    // Calculate time to sleep
    time_t end = time(nullptr);
    double delta = difftime(end, start);
    double sleep_time = 1.0 / FRAME_RATE - delta;
    if (sleep_time > 0) {
        timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = sleep_time * 1e9;
        nanosleep(&ts, nullptr);
    }

    return true;
}

int run(string savefile) {
    // Load game data
    GAME_DATA data = load(savefile);
    
    // Initialize ncurses
    initscr();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);

    // Main game loop
    render(data);
    while (update(data)) { }

    // Cleanup
    endwin();
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
