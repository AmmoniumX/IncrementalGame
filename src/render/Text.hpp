#pragma once

#include <string>
#include <variant>
#include <memory>
#include <optional>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <cwchar>

class Text {
private:
    int y, x;
    std::variant<std::string, std::wstring> text;
    int color_pair = 0;
    std::shared_ptr<WINDOW> win;
    bool needsClear = false;

    WINDOW* getWin() const {
        return win ? win.get() : stdscr;
    }

    void doClear(size_t len) {
        std::string blank(len, ' ');
        mvwaddstr(getWin(), y, x, blank.c_str());
    }

    void doClear() {
        if (needsClear) {
            doClear(getVisualLength());
            needsClear = false;
        }
    }

public:
    Text(int y, int x, const std::string& txt, int color_pair = 0, std::shared_ptr<WINDOW> win = nullptr)
        : y(y), x(x), text(txt), color_pair(color_pair), win(win) {}

    Text(int y, int x, const std::wstring& txt, int color_pair = 0, std::shared_ptr<WINDOW> win = nullptr)
        : y(y), x(x), text(txt), color_pair(color_pair), win(win) {}

    size_t getLength() const {
        return std::visit([](const auto& str) { return str.size(); }, text);
    }

    size_t getVisualLength() const {
        return std::visit([](const auto& str) {
            if constexpr (std::is_same_v<decltype(str), const std::wstring&>) {
                return static_cast<size_t>(wcswidth(str.c_str(), str.size()));
            } else {
                return str.size();
            }
        }, text);
    }

    bool isWide() const {
        return std::holds_alternative<std::wstring>(text);
    }

    bool isEmpty() const {
        return std::visit([](const auto& str) { return str.empty(); }, text);
    }

    std::optional<std::string> getText() const {
        if (auto p = std::get_if<std::string>(&text)) return *p;
        return std::nullopt;
    }

    std::optional<std::wstring> getWText() const {
        if (auto p = std::get_if<std::wstring>(&text)) return *p;
        return std::nullopt;
    }

    int getX() const { return x; }
    int getY() const { return y; }

    void setText(const std::string& newText, bool clear = false) {
        if (clear) needsClear = true;
        text = newText;
    }

    void setText(const std::wstring& newText, bool clear = false) {
        if (clear) needsClear = true;
        text = newText;
    }

    void setX(int px) { x = px; }
    void setY(int py) { y = py; }

    void setColorPair(int pair) {
        if (pair >= 0) {
            color_pair = pair;
        } else {
            std::fprintf(stderr, "Invalid color pair: %d\n", pair);
        }
    }

    int getColorPair() const { return color_pair; }

    void render() {
        doClear();
        if (isEmpty()) return;

        WINDOW* w = getWin();

        if (color_pair > 0) {
            wattron(w, COLOR_PAIR(color_pair));
        }

        std::visit([&](const auto& str) {
            using T = std::decay_t<decltype(str)>;
            if constexpr (std::is_same_v<T, std::string>) {
                mvwaddstr(w, y, x, str.c_str());
            } else if constexpr (std::is_same_v<T, std::wstring>) {
                mvwaddwstr(w, y, x, str.c_str());
            }
        }, text);

        if (color_pair > 0) {
            wattroff(w, COLOR_PAIR(color_pair));
        }
    }

    void clear() {
        needsClear = true;
    }

    void reset() {
        clear();
        std::visit([](auto& str) { str.clear(); }, text);
    }
};

using TextPtr = std::shared_ptr<Text>;
