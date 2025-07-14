#pragma once

#include <ncurses.h>
#include <string>
#include <iostream>
#include <print>

#include "../setup.hpp"
#include "./Text.hpp"
#include "./Window.hpp"
#include "./Screen.hpp"

/*
* @class ScreenManager
* @brief A class to manage the current screen and handle screen changes.
*/
class ScreenManager { // Singleton class
private:
    ScreenPtr currentScreen = nullptr;
    std::shared_ptr<Screen> nextScreen = nullptr;
    bool screenChange = false;
    bool exitRequested = false;

    // Private constructor for singleton
    ScreenManager() {};
    
    // Deleted copy constructor and assignment operator
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

public:
    // Static method to get the singleton instance
    static ScreenManager& getInstance() {
        static ScreenManager instance;
        return instance;
    }

    std::shared_ptr<Screen> getCurrentScreen() const { return currentScreen; }
    
    // void registerScreen(const std::shared_ptr<Screen> screen) {
    //     loadedScreens.push_back(screen);
    // }

    void changeScreen(const std::shared_ptr<Screen> screen) {
        nextScreen = screen;
        screenChange = true;
    }

    void requestExit() {
        exitRequested = true;
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
            currentScreen->render();
            char input = getch();
            exitRequested = currentScreen->tick(input);
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