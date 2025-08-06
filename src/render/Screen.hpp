#pragma once

#include <functional>
#include <ncursesw/ncurses.h>
#include <string>
#include <print>
#include <list>

#include "./Text.hpp"
#include "./Window.hpp"

/*
* @class Screen
* @brief A class to represent and render a screen with multiple windows and texts.
*/
class Screen {
private:
    std::list<Text> texts; // Screen-level texts
    std::list<Window> windows; // Screen-level windows

public:
    virtual ~Screen() = default;

    std::reference_wrapper<Text> putText(int y, int x, const std::string& text, int color_pair=0) {
        texts.emplace_back(y, x, text, color_pair);
        return std::ref(texts.back());
    }

    std::reference_wrapper<Window> createWindow(int y, int x, int width, int height, bool visible=true, int color_pair=0) {
        windows.emplace_back(x, y, width, height, visible, color_pair, nullptr);
        return std::ref(windows.back());
    }

    void render() {
        // clear();
        for (auto& text : texts) {
            text.render();
        }
        for (auto& window : windows) {
            window.render();
        }
        refresh();
    }

    virtual bool onTick([[maybe_unused]] const char input) {
        return false; // Default behavior: continue the game loop
    }
};

