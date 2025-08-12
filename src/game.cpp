#include <atomic>
#include <ctime>
#include <getopt.h>
#include <iostream>
#include <ncursesw/ncurses.h>
#include <print>
#include <thread>
#include <string>
#include <fstream>

#include "game.hpp"
#include "./SystemManager.hpp"
#include "./systems/ResourceManager.hpp"

// Constants
std::atomic_bool GameInternals::exit = false;
using nlohmann::json;

// Ncurses setup
void setupNcurses() {
    setlocale(LC_ALL, ""); // Enable UTF-8 support in ncurses
    initscr();             // Initialize ncurses mode

    // Check terminal color support
    if (has_colors()) {
        start_color();        // Start color functionality
        use_default_colors(); // Use default terminal colors
    }

    if (!has_colors() || COLORS < 256) {
        std::println(stderr,
                     "This terminal does not support 256-bit colors! ({})",
                     COLORS);
        endwin();
        exit(EXIT_FAILURE);
    }

    cbreak();              // Disable line buffering
    noecho();              // Disable echoing of typed characters
    nodelay(stdscr, TRUE); // Make getch non-blocking
    keypad(stdscr, TRUE);  // Enable special keys
    curs_set(0);           // Hide the cursor

    // Show supported colors
    std::println(stderr, "Supported colors: {}", COLORS);
    std::println(stderr, "Supported color pairs: {}", COLOR_PAIRS);

    // Initialize color pairs
    init_pair(GAME_COLORS::DEFAULT, COLOR_WHITE,
              -1); // -1 for default background
    init_pair(GAME_COLORS::YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
    init_pair(GAME_COLORS::RED_BLACK, COLOR_RED, COLOR_BLACK);
    init_pair(GAME_COLORS::WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(GAME_COLORS::GRAY_BLACK, 8, COLOR_BLACK);
    init_pair(GAME_COLORS::YELLOW_GRAY, COLOR_YELLOW, 8);
    init_pair(GAME_COLORS::RED_GRAY, COLOR_RED, 8);
}

// helper json method
template <typename T>
T get_or(const json &j, const std::string &key, const T &default_value) {
    if (j.contains(key)) {
        return j[key].get<T>();
    }
    return default_value;
}

// Convert game data to json
json to_json() { return ResourceManager::instance().serialize(); }

// Convert json to game data
void from_json(const json &j) { ResourceManager::instance().deserialize(j); }

// Save game data
void save(const string &filename) {
    std::println(stderr, "Saving game data to {}", filename);

    // Convert to json
    json j = to_json();
    std::ofstream o(filename);
    if (!o.is_open()) {
        std::println(stderr, "Error: Could not open file {}", filename);
        return;
    }
    o << j.dump(0) << std::endl;

    // std::println(stderr, "Game data saved!");
}

// Load game data
void load(const string &filename) {
    std::println(stderr, "Loading game data from {}", filename);

    // Load json from file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::println(stderr,
                     "File not found, ResourceManager will be empty!");
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception &e) {
        std::println(stderr,
                     "Error: Could not parse json! Is data corrupted? {}",
                     e.what());
        throw std::runtime_error("Could not parse json");
    }
    from_json(j);

    // std::println(stderr, "Game data loaded!");
    return;
}
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
