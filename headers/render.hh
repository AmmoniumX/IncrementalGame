#pragma once

#include <ncurses.h>
#include <functional>
#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <ctime>

#include "game.hh"

// Constants
const uint FRAME_RATE = 15;

// Function prototypes
void setupNcurses();
void renderText(const std::string& text, int x, int y, int colorPair);

// Ncurses setup
void setupNcurses() {
    initscr();              // Initialize ncurses mode
    cbreak();               // Disable line buffering
    noecho();               // Disable echoing of typed characters
    nodelay(stdscr, TRUE);  // Make getch non-blocking
    keypad(stdscr, TRUE);   // Enable special keys
    start_color();          // Enable color functionality
    curs_set(0);            // Hide the cursor
}

// Classes
class Text {
private:
    int y, x;
    std::string text;

public:
    Text(int y, int x, const std::string& text) : 
        y(y), x(x), text(text) {}

    std::string getText() const { return text; }
    int getX() const { return x; }
    int getY() const { return y; }

    void setText(const std::string& newText) { text = newText; }
    void setX(int px) { x = px; }
    void setY(int py) { y = py; }

    void render() {
        mvprintw(y, x, "%s", text.c_str());
    }
};
typedef std::shared_ptr<Text> TextPtr;

class Window {
private:
    std::shared_ptr<WINDOW> win;
    std::vector<std::shared_ptr<Text>> texts; // Window-level texts
public:
    Window(int x, int y, int width, int height) {
        win = std::shared_ptr<WINDOW>(newwin(height, width, y, x), [](WINDOW* w) { delwin(w); });
        box(win.get(), 0, 0);
    }

    void render() {
        for (const auto& text : texts) {
            text->render();
        }
        wrefresh(win.get());
    }

    std::shared_ptr<Text> putText(int y, int x, const std::string& text) {
        auto textObj = std::make_shared<Text>(y, x, text);
        texts.push_back(textObj);
        return textObj;
    }
};
typedef std::shared_ptr<Window> WindowPtr;

class Screen {
private:
    std::vector<std::shared_ptr<Text>> texts; // Screen-level texts
    std::vector<std::shared_ptr<Window>> windows; // Screen-level windows
    std::function<bool(GAME_DATA*, const char)> onTick = nullptr;

public:
    void setOnTick(const std::function<bool(GAME_DATA*, const char)> onTick) {
        this->onTick = onTick;
    }

    std::shared_ptr<Text> putText(int y, int x, const std::string& text) {
        auto textObj = std::make_shared<Text>(y, x, text);
        texts.push_back(textObj);
        return textObj;
    }

    std::shared_ptr<Window> createWindow(int y, int x, int width, int height) {
        auto window = std::make_shared<Window>(x, y, width, height);
        windows.push_back(window);
        return window;
    }

    void render() {
        for (const auto& text : texts) {
            text->render();
        }
        for (const auto& window : windows) {
            window->render();
        }
        refresh();
    }

    bool tick(GAME_DATA *data, char input) {
        // Return true if exit is requested
        if (onTick) {
            return onTick(data, input);
        }
        return false;
    }
};
typedef std::shared_ptr<Screen> ScreenPtr;

class ScreenManager { // Singleton class
private:
    // std::vector<ScreenPtr> loadedScreens;
    ScreenPtr currentScreen = nullptr;
    bool screenChange = false;
    bool exitRequested = false;
    std::shared_ptr<Screen> nextScreen = nullptr;
    GAME_DATA *data = nullptr;
    ScreenManager() {} // Private constructor for singleton pattern
public:
    // Deleted copy constructor and assignment operator
    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    // Static method to get the singleton instance
    static ScreenManager& getInstance() {
        static ScreenManager instance;
        return instance;
    }

    void initialize(GAME_DATA *data, const ScreenPtr screen) {
        if (this->data || this->currentScreen) throw std::runtime_error("ScreenManager already initialized");
        this->data = data;
        this->currentScreen = screen;
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
        if (!data || !currentScreen) throw std::runtime_error("ScreenManager is not initialized!");

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
            exitRequested = currentScreen->tick(data, input);
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
