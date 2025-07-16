#pragma once

#include <ncursesw/ncurses.h>
#include <string>
#include <memory>
#include <iostream>
#include <print>
#include <optional>
#include <variant>

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
    std::variant<std::string, std::wstring> text; // Support for wide characters
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
        doClear(static_cast<int>(getLength()));
    }

public:
    Text(int y, int x, const std::string& text, int color_pair = 0, std::shared_ptr<WINDOW> win=nullptr) : 
        y(y), x(x), text(text), color_pair(color_pair), win(win) {}

    Text(int y, int x, const std::wstring& text, int color_pair = 0, std::shared_ptr<WINDOW> win=nullptr) :
        y(y), x(x), text(text), color_pair(color_pair), win(win) {}

    size_t getLength() const {
        return std::visit([](const auto& str) { return str.size(); }, text);
    }

    bool isWide() const {
        return std::holds_alternative<std::wstring>(text);
    }

    bool isEmpty() const {
        return std::visit([](const auto& str) { return str.empty(); }, text);
    }

    std::optional<std::string> getText() const {
        if (std::holds_alternative<std::string>(text)) {
            return std::get<std::string>(text);
        }
        return std::nullopt;
    }

    std::optional<std::wstring> getWText() const {
        if (std::holds_alternative<std::wstring>(text)) {
            return std::get<std::wstring>(text);
        }
        return std::nullopt;
    }

    int getX() const { return x; }
    int getY() const { return y; }

    void setText(const std::string& newText, bool clear=false) { 
        if (clear) { needsClear = static_cast<int>(getLength()); }
        text = newText;
    }

    void setText(const std::wstring& newText, bool clear=false) { 
        if (clear) { needsClear = static_cast<int>(getLength()); }
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
        if (isEmpty()) { return; }

        // Apply color before rendering text
        if (color_pair > 0) {
            if (!win) {
                attron(COLOR_PAIR(color_pair));
                if (isWide()) {
                    mvaddwstr(y, x, std::get<std::wstring>(text).c_str());
                } else {
                    mvaddstr(y, x, std::get<std::string>(text).c_str());
                }
                attroff(COLOR_PAIR(color_pair));
            } else {
                wattron(win.get(), COLOR_PAIR(color_pair));
                if (isWide()) {
                    mvwaddwstr(win.get(), y, x, std::get<std::wstring>(text).c_str());
                } else {
                    mvwaddstr(win.get(), y, x, std::get<std::string>(text).c_str());
                }
                wattroff(win.get(), COLOR_PAIR(color_pair));
            }
        } else {
            // Render without color
            if (!win) {
                if (isWide()) {
                    mvaddwstr(y, x, std::get<std::wstring>(text).c_str());
                } else {
                    mvaddstr(y, x, std::get<std::string>(text).c_str());
                }
            } else {
                if (isWide()) {
                    mvwaddwstr(win.get(), y, x, std::get<std::wstring>(text).c_str());
                } else {
                    mvwaddstr(win.get(), y, x, std::get<std::string>(text).c_str());
                }
            }
        }
    }

    void clear() {
        needsClear = static_cast<int>(getLength());
    }

    void reset() {
        clear();
        text = "";
    }

};
typedef std::shared_ptr<Text> TextPtr;