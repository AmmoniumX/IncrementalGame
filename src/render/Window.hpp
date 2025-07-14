#pragma once

#include <ncurses.h>
#include <string>
#include <memory>
#include <iostream>
#include <print>
#include <vector>
#include <functional>

#include "../setup.hpp"
#include "./Text.hpp"

/*
* @class Window
* @brief A class to represent and render a subwindow on the screen.
*
* @param x The x-coordinate of the window position.
* @param y The y-coordinate of the window position.
* @param width The width of the window.
* @param height The height of the window.
* @param visible Whether the window is currently visible or not.
* @param color_pair The color pair to use for the window background (default is 0, which means no color).
*/
class Window {
private:
    std::shared_ptr<WINDOW> win;
    std::vector<std::shared_ptr<Text>> texts; // Window-level texts
    std::vector<std::shared_ptr<Window>> subwindows; // Subwindows within this window
    int x, y, width, height;
    bool visible;
    int color_pair;
    std::shared_ptr<WINDOW> parentWin;
public:
    Window(int x, int y, int width, int height, bool visible, int color_pair=0, std::shared_ptr<WINDOW> parentWin=nullptr) 
    : x(x), y(y), width(width), height(height), visible(visible), color_pair(color_pair), parentWin(parentWin) {
        win = std::shared_ptr<WINDOW>(newwin(height, width, y, x), [](WINDOW* w) { delwin(w); });

        if (color_pair > 0) {
            wbkgd(win.get(), COLOR_PAIR(color_pair)); // Set background color if color is set
        }
    }

    bool isVisible() const { return visible; }
    void clearWindow() {
        // Temporarily set window color to the parent window's color
        if (parentWin) {
            wbkgd(win.get(), getbkgd(parentWin.get()));
        } else {
            wbkgd(win.get(), COLOR_PAIR(GAME_COLORS::DEFAULT)); // Default background
        }
        werase(win.get()); // Clear the window
        wrefresh(win.get()); // Refresh to apply changes
        // Restore the original color
        wbkgd(win.get(), COLOR_PAIR(color_pair));
        // Overwrite the text with spaces
        for (const auto& text : texts) {
            text->clear();
        }
    }
    void enable() { 
        visible = true; 
    }
    void disable() { 
        clearWindow();
        visible = false; 
    }
    void toggle() { visible ? disable() : enable(); }

    void render() {
        if (!win) {
            std::println(std::cerr, "Window is not initialized!");
            return;
        }
        if (!visible) return;

        onTick(); // Call ticker

        box(win.get(), 0, 0); // Draw the border

        // Render the texts
        for (const auto& text : texts) {
            text->render();
        }

        // Render subwindows
        for (const auto& subwindow : subwindows) {
            subwindow->render();
        }

        // Refresh the window
        wrefresh(win.get());
    }

    std::shared_ptr<Text> putText(int textY, int textX, const std::string& text, int text_color_pair=0) {
        auto textObj = std::make_shared<Text>(textY, textX, text, text_color_pair, win);
        texts.push_back(textObj);
        return textObj;
    }

    std::shared_ptr<Window> createSubwindow(int subY, int subX, int subWidth, int subHeight, 
        bool visible=true, int color_pair=0) {

        auto subwindow = std::make_shared<Window>(subX, subY, subWidth, subHeight, visible, color_pair, win);
        subwindows.push_back(subwindow);
        return subwindow;
    }

    virtual void onTick() {}
};
typedef std::shared_ptr<Window> WindowPtr;