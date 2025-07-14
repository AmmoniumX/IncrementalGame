#pragma once

#include <ncurses.h>
#include <string>
#include <memory>
#include <iostream>
#include <print>

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
            std::println(std::cerr, "Invalid color pair: {}", pair);
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