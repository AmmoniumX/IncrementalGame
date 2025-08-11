#pragma once

#include <functional>
#include <ncursesw/ncurses.h>
#include <print>
#include <list>
#include <string_view>

#include "./setup.hpp"
#include "./SystemManager.hpp"
#include "./render/Screen.hpp"

using namespace std::literals::string_view_literals;

/*
* @class ScreenManager
* @brief A class to manage the current screen and handle screen changes.
*/
class ScreenManager : public RegisteredSystem<ScreenManager> { // Singleton class
private:
    Screen *currentScreen = nullptr;
    Screen *nextScreen = nullptr;
    std::list<std::unique_ptr<Screen>> screens;
    bool screenChange = false;

    // Private constructor for singleton
    ScreenManager() {};
    
    // Deleted copy constructor and assignment operator
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

public:
    static constexpr std::string_view RESOURCE_ID = "ScreenManager"sv;

    static ScreenManager &instance() {
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

    char getInput() {
        return getch();
    }

    void onTick() override {
        // Set the screen on the first run
        if (!currentScreen && nextScreen) {
            currentScreen = nextScreen;
            nextScreen = nullptr;
            screenChange = false;
        }

        // Throw if there is no screen set
        if (!currentScreen) throw std::runtime_error("ScreenManager is not initialized!");

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
    }

    virtual ~ScreenManager() override {
        endwin();
    }
};
