#pragma once

#include <ncursesw/ncurses.h>
#include <string>
#include <print>
#include <vector>
#include <memory>

#include "./Text.hpp"
#include "./Window.hpp"

/*
* @class Screen
* @brief A class to represent and render a screen with multiple windows and texts.
*/
class Screen {
private:
    std::vector<std::unique_ptr<Text>> texts; // Screen-level texts
    std::vector<std::unique_ptr<Window>> windows; // Screen-level windows

public:
    virtual ~Screen() = default;

    Text &putText(int y, int x, const std::string& text, int color_pair=0) {
        texts.push_back(std::make_unique<Text>(y, x, text, color_pair));
        return *texts.back();
    }

    Window &createWindow(int y, int x, int width, int height, bool visible=true, int color_pair=0) {
        windows.push_back(std::make_unique<Window>(x, y, width, height, visible, color_pair, nullptr));
        return *windows.back();
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

    virtual void onTick() {}
};

