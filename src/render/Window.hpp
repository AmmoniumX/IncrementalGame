#pragma once

#include <string>
#include <memory>
#include <initializer_list>
#include <print>
#include <vector>
#include <span>
#include <functional>

#include <curses.h>

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
        void operator()(WINDOW* win) const;
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
    static WinUniqPtr newWin(int height, int width, int y, int x);

    virtual ~Window() = default;

    enum class Alignment {
        LEFT,
        CENTER,
        RIGHT
    };
    
    std::reference_wrapper<Text> setTitle(const std::string& text, Alignment alignment=Alignment::LEFT, int color_pair=0, int offset=0);

    Window(int x, int y, int width, int height, bool visible, int color_pair=0, WINDOW *parentWin=nullptr);
    // move constructor for allowing subwindows.emplace_back()
    Window(Window&& other) noexcept;

    // move assignment operator
    Window& operator=(Window&& other) noexcept;
    
    // Delete copy constructor and copy assignment
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void setColorPair(int col);

    bool isVisible() const;
    void clearWindow();
    void enable();
    void disable();
    void toggle();

    void render();

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
        bool visible=true, int color_pair=0);

    virtual void onTick();
};

