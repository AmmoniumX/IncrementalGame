#pragma once

#include <ncursesw/ncurses.h>
#include <string>
#include <memory>
#include <initializer_list>
#include <print>
#include <vector>
#include <span>
#include <functional>

#include "../game.hpp"
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
public:
    struct WindowDeleter {
        void operator()(WINDOW* win) const {
            if (win) {
                delwin(win);
            }
        }
    };
    typedef std::unique_ptr<WINDOW, WindowDeleter> WinUniqPtr;
private:
    WinUniqPtr win;
    std::vector<std::unique_ptr<Text>> texts; // Window-level texts
    std::vector<std::unique_ptr<Window>> subwindows; // Subwindows within this window
    [[maybe_unused]] int x, y, width, height;
    bool visible;
    int color_pair;
    WINDOW *parentWin = nullptr;
    Text *title = nullptr;
public:
    static WinUniqPtr newWin(int height, int width, int y, int x) {
        return WinUniqPtr(newwin(height, width, y, x));
    }

    virtual ~Window() = default;

    enum class Alignment {
        LEFT,
        CENTER,
        RIGHT
    };
    
    std::reference_wrapper<Text> setTitle(const std::string& text, Alignment alignment=Alignment::LEFT, int color_pair=0, int offset=0) {
        if (title) {
            title->setText(text, true, color_pair);
        } else {
            title = &putText(0, 0, text, color_pair);
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
        return std::ref(*title);
    }

    Window(int x, int y, int width, int height, bool visible, int color_pair=0, WINDOW *parentWin=nullptr)
    : win(newWin(height, width, y, x)), x(x), y(y), width(width), height(height), visible(visible),
      color_pair(color_pair), parentWin(parentWin)
    {
        if (color_pair > 0) {
            wbkgd(win.get(), COLOR_PAIR(color_pair));
        }
    }

    // move constructor for allowing subwindows.emplace_back()
    Window(Window&& other) noexcept 
    : win(std::move(other.win)), x(other.x), y(other.y), width(other.width), height(other.height), 
      visible(other.visible), color_pair(other.color_pair), parentWin(other.parentWin)
    {
        if (color_pair > 0) {
            wbkgd(win.get(), COLOR_PAIR(color_pair));
        }
    }

    // move assignment operator
    Window& operator=(Window&& other) noexcept {
        if (this != &other) {
            win = std::move(other.win);
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            visible = other.visible;
            color_pair = other.color_pair;
            parentWin = other.parentWin;
        }
        return *this;
    }
    
    // Delete copy constructor and copy assignment
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void setColorPair(int col) {
        color_pair = col;
        wbkgd(win.get(), COLOR_PAIR(color_pair));
    }

    bool isVisible() const { return visible; }
    void clearWindow() {
        // Temporarily set window color to the parent window's color
        if (parentWin) {
            wbkgd(win.get(), getbkgd(parentWin));
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
            std::println(stderr, "Window is not initialized!");
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

    template<TextString T>
    Text &putText(int textY, int textX, const T& text, int text_color_pair=0) {
        texts.push_back(std::make_unique<Text>(textY, textX, text, text_color_pair, win.get()));
        return *texts.back();
    }

    template<TextString T>
    Text &putText(int textY, int textX, const std::span<const Text::TextChunk<T>> chunks) {
        texts.push_back(std::make_unique<Text>(textY, textX, chunks, win.get()));
        return *texts.back();
    }

    template<TextString T>
    std::reference_wrapper<Text> putText(int textY, int textX, const std::initializer_list<const Text::TextChunk<T>> chunks) {
        texts.push_back(std::make_unique<Text>(textY, textX, chunks, win.get()));
        return *texts.back();
    }

    Window &createSubwindow(int subY, int subX, int subWidth, int subHeight, 
        bool visible=true, int color_pair=0) {
        subwindows.push_back(std::make_unique<Window>(subX, subY, subWidth, subHeight, visible, color_pair, win.get()));
        return *subwindows.back();
    }

    virtual void onTick() {}
};

