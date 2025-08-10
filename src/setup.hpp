#pragma once

#include <iostream>
#include <print>

#include <ncursesw/ncurses.h>

// Constants
constexpr uint FRAME_RATE = 30;

enum GAME_COLORS {
    DEFAULT = 0,
    YELLOW_BLACK = 1,
    RED_BLACK = 2,
    WHITE_BLACK = 3,
    GRAY_BLACK = 4,
    YELLOW_GRAY = 5,
    RED_GRAY = 6
};

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