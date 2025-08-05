#pragma once

#include <ncursesw/ncurses.h>
#include <string>
#include <memory>
#include <print>
#include <vector>

#include "./Text.hpp"
#include "./Window.hpp"

/*
* @class Screen
* @brief A class to represent and render a screen with multiple windows and texts.
*/
class Screen {
private:
    std::vector<std::shared_ptr<Text>> texts; // Screen-level texts
    std::vector<std::shared_ptr<Window>> windows; // Screen-level windows

public:
    std::shared_ptr<Text> putText(int y, int x, const std::string& text, int color_pair=0) {
        auto textObj = std::make_shared<Text>(y, x, text, color_pair);
        texts.push_back(textObj);
        return textObj;
    }

    std::shared_ptr<Window> createWindow(int y, int x, int width, int height, bool visible=true, int color_pair=0) {
        auto window = std::make_shared<Window>(x, y, width, height, visible, color_pair);
        windows.push_back(window);
        return window;
    }

    void render() {
        // clear();
        for (const auto& text : texts) {
            text->render();
        }
        for (const auto& window : windows) {
            window->render();
        }
        refresh();
    }

    virtual bool onTick([[maybe_unused]] const char input) {
        return false; // Default behavior: continue the game loop
    }
};
typedef std::shared_ptr<Screen> ScreenPtr;
