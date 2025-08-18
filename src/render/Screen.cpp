#include <curses.h>

#include "Screen.hpp"

Text &Screen::putText(int y, int x, const std::string& text, int color_pair) {
    texts.push_back(std::make_unique<Text>(y, x, text, color_pair));
    return *texts.back();
}

Window &Screen::createWindow(int y, int x, int width, int height, bool visible, int color_pair) {
    windows.push_back(std::make_unique<Window>(x, y, width, height, visible, color_pair, nullptr));
    return *windows.back();
}

void Screen::render() {
    // clear();
    for (const auto& text : texts) {
        text->render();
    }
    for (const auto& window : windows) {
        window->render();
    }
    refresh();
}

void Screen::onTick() {}
