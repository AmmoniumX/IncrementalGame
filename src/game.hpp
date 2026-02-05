#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <string_view>

#include "../include/json.hpp"
#include "BigNum.hpp"

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = Clock::duration;
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

namespace Game {
extern std::atomic_bool exit;
} // namespace Game

// Constants
static inline constexpr int TARGET_TPS = 30;
static inline constexpr std::chrono::duration TARGET_TICK_TIME =
    1s / TARGET_TPS;
