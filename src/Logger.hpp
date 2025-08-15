#pragma once

#include <fstream>
#include <format>
#include <print>

class Logger {
private:
    static std::ofstream &out();
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
public:

    // static Logger &instance();

    template <class... Args>
    static void print(std::format_string<Args...> fmt, Args&&... args) {
        return std::print(out(), fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    static void println(std::format_string<Args...> fmt, Args&&... args) {
        return std::println(out(), fmt, std::forward<Args>(args)...);
    }
};
