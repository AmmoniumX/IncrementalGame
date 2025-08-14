#pragma once

#include <atomic>
#include <string>
#include <print>

#ifdef _WIN32
    #include <curses.h> // from PDCurses
#else
    #include <ncurses.h>
#endif
// #include <ncursesw/ncurses.h>

#include "BigNum.hpp"
#include "json.hpp"

using nlohmann::json;
using std::string;
using N = BigNum;

enum GAME_COLORS {
    DEFAULT = 0,
    YELLOW_BLACK = 1,
    RED_BLACK = 2,
    WHITE_BLACK = 3,
    GRAY_BLACK = 4,
    YELLOW_GRAY = 5,
    RED_GRAY = 6
};

namespace GameInternals {
    extern std::atomic_bool exit;
} // namespace GameInternals

// Constants
static inline const uint FRAME_RATE = 30;
static inline const int GAME_TICK_SPEED = 30;

