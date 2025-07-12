#pragma once

#include <ncurses.h>
#include <functional>
#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <ctime>

// Constants
constexpr uint FRAME_RATE = 30;

namespace GAME_COLORS {
    constexpr int DEFAULT = 0; // Default color pair
    constexpr int YELLOW_BLACK = 1; // Yellow on black
    constexpr int RED_BLACK = 2; // Red on black
    constexpr int WHITE_BLACK = 3; // White on black
}

// Ncurses setup
void setupNcurses() {
    initscr();              // Initialize ncurses mode
    cbreak();               // Disable line buffering
    noecho();               // Disable echoing of typed characters
    nodelay(stdscr, TRUE);  // Make getch non-blocking
    keypad(stdscr, TRUE);   // Enable special keys
    start_color();          // Enable color functionality
    curs_set(0);            // Hide the cursor

    // Initialize color pairs
    init_pair(GAME_COLORS::YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
    init_pair(GAME_COLORS::RED_BLACK, COLOR_RED, COLOR_BLACK);
    init_pair(GAME_COLORS::WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
}

/*
 * @class Text
 * @brief A class to represent and render text on a window or the standard screen.
 *
 * @param y The y-coordinate of the text position.
 * @param x The x-coordinate of the text position.
 * @param text The string content of the text.
 * @param win A shared pointer to the window where the text will be rendered. If nullptr, the text will be rendered on the standard screen.
 */
class Text {
private:
    int y, x;
    std::string text;
    int color_pair = 0;
    std::shared_ptr<WINDOW> win;
    int needsClear = 0;
    void doClear(int len) {
        if (!win) {
            mvprintw(y, x, "%*s", len, " ");
        } else {
            mvwprintw(win.get(), y, x, "%*s", len, " ");
        }
    }
    
    void doClear() {
        doClear(static_cast<int>(text.size()));
    }
public:
    Text(int y, int x, const std::string& text, int color_pair = 0, std::shared_ptr<WINDOW> win=nullptr) : 
        y(y), x(x), text(text), color_pair(color_pair), win(win) {}

    std::string getText() const { return text; }
    int getX() const { return x; }
    int getY() const { return y; }

    void setText(const std::string& newText, bool clear=false) { 
        if (clear) { needsClear = static_cast<int>(text.size()); }
        text = newText;
    }
    void setX(int px) { x = px; }
    void setY(int py) { y = py; }

    void setColorPair(int pair) { 
        if (pair < 0) {
            std::cerr << "Invalid color pair: " << pair << std::endl;
            return;
        }
        color_pair = pair; 
    }
    int getColorPair() const { return color_pair; }

    void render() {
        if (needsClear > 0) {
            doClear(needsClear);
            needsClear = 0;
        }
        if (text.empty()) { return; }

        // Apply color before rendering text
        if (color_pair > 0) {
            if (!win) {
                attron(COLOR_PAIR(color_pair));
                mvprintw(y, x, "%s", text.c_str());
                attroff(COLOR_PAIR(color_pair));
            } else {
                wattron(win.get(), COLOR_PAIR(color_pair));
                mvwprintw(win.get(), y, x, "%s", text.c_str());
                wattroff(win.get(), COLOR_PAIR(color_pair));
            }
        } else {
            // Render without color
            if (!win) {
                mvprintw(y, x, "%s", text.c_str());
            } else {
                mvwprintw(win.get(), y, x, "%s", text.c_str());
            }
        }
    }

    void clear() {
        needsClear = static_cast<int>(text.size());
    }

    void reset() {
        clear();
        text = "";
    }

};
typedef std::shared_ptr<Text> TextPtr;

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
    int x, y, width, height;
    bool visible;
    int color_pair;
public:
    Window(int x, int y, int width, int height, bool visible, int color_pair=0) 
    : x(x), y(y), width(width), height(height), visible(visible), color_pair(color_pair) {
        win = std::shared_ptr<WINDOW>(newwin(height, width, y, x), [](WINDOW* w) { delwin(w); });

        if (color_pair > 0) {
            wbkgd(win.get(), COLOR_PAIR(color_pair)); // Set background color if color is set
        }
    }

    bool isVisible() const { return visible; }
    void clearWindow() {
        werase(win.get()); // Clear the window
        wrefresh(win.get()); // Refresh to apply changes

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
        if (!visible) return;

        if (!win) {
            std::cerr << "Window is not initialized!" << std::endl;
            return;
        }

        // Draw the border
        box(win.get(), 0, 0);

        // Render the texts
        for (const auto& text : texts) {
            text->render();
        }

        // Refresh the window
        wrefresh(win.get());
    }

    std::shared_ptr<Text> putText(int textY, int textX, const std::string& text, int text_color_pair=0) {
        auto textObj = std::make_shared<Text>(textY, textX, text, text_color_pair, win);
        texts.push_back(textObj);
        return textObj;
    }
};
typedef std::shared_ptr<Window> WindowPtr;

/*
* @class Screen
* @brief A class to represent and render a screen with multiple windows and texts.
*/
class Screen {
private:
    std::vector<std::shared_ptr<Text>> texts; // Screen-level texts
    std::vector<std::shared_ptr<Window>> windows; // Screen-level windows
    std::function<bool(const char)> onTick = nullptr;

public:
    void setOnTick(const std::function<bool(const char)> onTick) {
        this->onTick = onTick;
    }

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

    bool tick(char input) {
        // Return true if exit is requested
        if (onTick) {
            return onTick(input);
        }
        return false;
    }
};
typedef std::shared_ptr<Screen> ScreenPtr;

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
