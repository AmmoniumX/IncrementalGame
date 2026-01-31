#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <thread>

#include <curses.h>

#include "./SystemManager.hpp"
#include "./resources/SaveData.hpp"
#include "Logger.hpp"
#include "game.hpp"

namespace fs = std::filesystem;

// Constants
std::atomic_bool Game::exit = false;
using nlohmann::json;
using Clock = std::chrono::steady_clock;
using namespace std::chrono_literals;
namespace detail {
std::ofstream logstream;
}
std::ofstream &Logger::out() { return detail::logstream; }

// curses setup
void setupNcurses() {
  setlocale(LC_ALL, ""); // Enable UTF-8 support in
  initscr();

  // Check terminal color support
  if (has_colors()) {
    start_color();        // Start color functionality
    use_default_colors(); // Use default terminal colors
  }

  if (!has_colors() || COLORS < 256) {
    Logger::println("This terminal does not support 256-bit colors! ({})",
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
  Logger::println("Supported colors: {}", COLORS);
  Logger::println("Supported color pairs: {}", COLOR_PAIRS);

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

void gameTick() { SystemManager::instance().onTick(); }

void run() {
  Logger::println("Running game...");
  // Main game loop
  do {
    auto start = Clock::now();
    gameTick();
    auto end = Clock::now();
    auto delta = end - start;
    auto sleep_time = TARGET_TICK_TIME - delta;
    if (sleep_time > 0ms) {
      std::this_thread::sleep_for(sleep_time);
    }
  } while (!Game::exit);
  Logger::println("Exiting...");
}

void init(fs::path savepath) {

  // Initialize curses
  Logger::println("Initializing curses...");
  setupNcurses();

  // Initialize systems
  SystemManager::init();

  // Load game data
  if (fs::is_regular_file(savepath)) {
    std::ifstream file(savepath);
    SaveData::instance().deserialize(file);
  }
}

void cleanup(fs::path savepath) {
  // Save game data
  std::ofstream file(savepath);
  SaveData::instance().serialize(file);
}

void ensure_directory(fs::path directory) {
  if (fs::exists(directory) && !fs::is_directory(directory)) {
    std::println(stderr, "Directory path exists but isn't a directory: ",
                 directory.string());
    std::exit(EXIT_FAILURE);
  } else if (!fs::exists(directory)) {
    try {
      (void)fs::create_directory(directory);
    } catch (fs::filesystem_error &ex) {
      std::println(stderr, "Error creating directory {}: {}",
                   directory.string(), ex.what());
      std::exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {
  string savefile = "save.json";

  // Loop through the command-line arguments starting from the first
  // user-provided argument (at index 1), since argv[0] is the program name.
  for (int i = 1; i < argc; ++i) {
    // Convert the current C-style char* argument to a C++ std::string
    // for easier comparison.
    std::string arg = argv[i];

    // Check if the current argument is the "--save" flag.
    if (arg == "--save") {
      // Check if there is a next argument to use as the value.
      if (i + 1 < argc) {
        // The next argument is the path to the save file.
        savefile = argv[i + 1];
        i++; // Skip the next argument as it's the value for "--save".
      } else {
        // If "--save" is the last argument, there's a missing value.
        std::cerr << "Error: --save option requires an argument." << std::endl;
        std::cerr << "Usage: " << argv[0] << " [--save <savefile>]"
                  << std::endl;
        return EXIT_FAILURE;
      }
    } else {
      // If the argument is not "--save", it's an unrecognized option.
      std::cerr << "Error: Unrecognized option '" << arg << "'" << std::endl;
      std::cerr << "Usage: " << argv[0] << " [--save <savefile>]" << std::endl;
      return EXIT_FAILURE;
    }
  }

  fs::path logdir("./logs/");
  ensure_directory(logdir);
  fs::path savedir("./saves/");
  ensure_directory(savedir);
  fs::path savepath = savedir / savefile;
  detail::logstream.open("./logs/latest.log");

  // Setup
  init(savepath);
  run();
  cleanup(savepath);

  detail::logstream.close();
  return EXIT_SUCCESS;
}
