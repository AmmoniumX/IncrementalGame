#pragma once

#include <functional>
#include <ncursesw/ncurses.h>
#include <print>
#include <list>

#include "../setup.hpp"
#include "./Screen.hpp"

/*
* @class ScreenManager
* @brief A class to manage the current screen and handle screen changes.
*/
class ScreenManager { // Singleton class
private:
    Screen *currentScreen = nullptr;
    Screen *nextScreen = nullptr;
    std::list<std::unique_ptr<Screen>> screens;
    bool screenChange = false;
    bool exitRequested = false;

    // Private constructor for singleton
    ScreenManager() {};
    
    // Deleted copy constructor and assignment operator
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

public:
    // Static method to get the singleton instance
    static ScreenManager& instance() {
        static ScreenManager instance;
        return instance;
    }

    Screen *getCurrentScreen() const { return currentScreen; }
    
    std::reference_wrapper<Screen> registerScreen(std::unique_ptr<Screen> &&screen) {
        screens.push_back(std::move(screen));
        return std::ref(*screens.back().get());
    }

    void changeScreen(Screen *screen) {
        nextScreen = screen;
        screenChange = true;
    }

    void requestExit() {
        exitRequested = true;
    }

    char getInput() {
        return getch();
    }

    void run() {
        // Set the screen on the first run
        if (!currentScreen && nextScreen) {
            currentScreen = nextScreen;
            nextScreen = nullptr;
            screenChange = false;
        }

        // Throw if there is no screen set
        if (!currentScreen) throw std::runtime_error("ScreenManager is not initialized!");

        do {
            // Run a single frame
            time_t start = time(nullptr);
            if (screenChange && nextScreen) {
                currentScreen = nextScreen;
                nextScreen = nullptr;
                screenChange = false;
            }
            currentScreen->onTick();
            currentScreen->render();
            time_t end = time(nullptr);

            // Calculate time to sleep
            double delta = difftime(end, start);
            double sleep_time = 1.0 / FRAME_RATE - delta;
            if (sleep_time > 0) {
                timespec ts;
                ts.tv_sec = 0;
                ts.tv_nsec = sleep_time * 1e9;
                nanosleep(&ts, nullptr);
            }
        } while (!exitRequested);
        endwin();
    }
};
