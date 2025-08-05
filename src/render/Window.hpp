#pragma once

#include <ncursesw/ncurses.h>
#include <string>
#include <memory>
#include <iostream>
#include <initializer_list>
#include <print>
#include <vector>
#include <span>

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
    TextPtr title = nullptr;
public:

    virtual ~Window() = default;

    enum class Alignment {
        LEFT,
        CENTER,
        RIGHT
    };
    
    TextPtr setTitle(const std::string& text, Alignment alignment=Alignment::LEFT, int color_pair=0, int offset=0) {
        if (title) {
            title->setText(text, true, color_pair);
        } else {
            title = putText(0, 0, text, color_pair);
        }
        switch (alignment) {
            case Alignment::LEFT:
                title->setX(1 + offset);
                break;
            case Alignment::CENTER: {
                int centerX = (width - static_cast<int>(text.size())) / 2 + offset;
                title->setX(centerX);
                break;
            }
            case Alignment::RIGHT: {
                int rightX = width - static_cast<int>(text.size()) - 1 - offset;
                title->setX(rightX);
                break;
            }
        }
        title->setY(0);
        return title;
    }

    Window(int x, int y, int width, int height, bool visible, int color_pair=0, std::shared_ptr<WINDOW> parentWin=nullptr) 
    : x(x), y(y), width(width), height(height), visible(visible), color_pair(color_pair), parentWin(parentWin) {
        win = std::shared_ptr<WINDOW>(newwin(height, width, y, x), [](WINDOW* w) { delwin(w); });

        if (color_pair > 0) {
            wbkgd(win.get(), COLOR_PAIR(color_pair)); // Set background color if color is set
        }
    }

    void setColorPair(int col) {
        color_pair = col;
        wbkgd(win.get(), COLOR_PAIR(color_pair));
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

        // Render the title if it exists
        if (title) {
            title->render();
        }

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
        auto t = std::make_shared<Text>(textY, textX, text, text_color_pair, win);
        texts.push_back(t);
        return t;
    }

    std::shared_ptr<Text> putText(int textY, int textX, const std::wstring& text, int text_color_pair=0) {
        auto t = std::make_shared<Text>(textY, textX, text, text_color_pair, win);
        texts.push_back(t);
        return t;
    }

    template<TextString T>
    std::shared_ptr<Text> putText(int textY, int textX, const std::span<const Text::TextChunk<T>> chunks) {
        auto t = std::make_shared<Text>(textY, textX, chunks, win);
        texts.push_back(t);
        return t;
    }

    template<TextString T>
    std::shared_ptr<Text> putText(int textY, int textX, const std::initializer_list<const Text::TextChunk<T>> chunks) {
        auto t = std::make_shared<Text>(textY, textX, chunks, win);
        texts.push_back(t);
        return t;
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
