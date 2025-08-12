#pragma once

#include <functional>
#include <ncursesw/ncurses.h>
#include <print>
#include <list>
#include <string_view>

#include "../SystemManager.hpp"
#include "../render/Screen.hpp"

using namespace std::literals::string_view_literals;

/*
* @class ScreenManager
* @brief A class to manage the current screen and handle screen changes.
*/
class ScreenManager : public System { // Singleton class
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

    static void init();

    static ScreenManager &instance();

    Screen *getCurrentScreen() const;    

    std::reference_wrapper<Screen> registerScreen(std::unique_ptr<Screen> &&screen);

    void changeScreen(Screen *screen);

    char getInput();

    void onTick() override;

    virtual ~ScreenManager() override;
};
