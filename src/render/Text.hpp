#pragma once

#include <cstddef>
#include <cstdio>
#include <algorithm>
#include <string>
#include <variant>
#include <vector>

#ifdef _WIN32
    #include <curses.h> // from PDCurses
#else
    #include <ncurses.h>
#endif

// #include <ncursesw/ncurses.h>
#include <cwchar>
#include <print>
#include <ranges>
#include <numeric>
#include <concepts>
#include <span>
#include <initializer_list>

template<typename T>
concept TextString = std::same_as<T, std::string> || std::same_as<T, std::wstring>;

class Text {
public:
    template<TextString str_t>
    struct TextChunk {
        int color_pair = 0;
        str_t text = str_t();
    };
private:
    int y, x;

    using TextChunks = std::variant<std::vector<TextChunk<std::string>>, std::vector<TextChunk<std::wstring>>>;

    TextChunks textChunks;

    WINDOW *win = stdscr;
    size_t needsClear = 0;
    bool clearStr = false;

    void doClear();

    static size_t getVisualLengthOf(TextChunk<std::string> chunk);

    static size_t getVisualLengthOf(TextChunk<std::wstring> chunk);

public:
    template<TextString T>
    Text(int y, int x, const T& txt, int color_pair = 0, WINDOW *win = stdscr)
        : y(y), x(x), textChunks(std::vector<TextChunk<T>>({{color_pair, txt}})), win(win) {}

    template<TextString T>
    Text(int y, int x, const std::span<const TextChunk<T>>& chunks, WINDOW *win = stdscr)
        : y(y), x(x), textChunks(std::vector<TextChunk<T>>(chunks.begin(), chunks.end())), win(win) {}

    template<TextString T>
    Text(int y, int x, const std::initializer_list<const TextChunk<T>>& chunks, WINDOW *win = stdscr)
        : y(y), x(x), textChunks(std::vector<TextChunk<T>>(chunks.begin(), chunks.end())), win(win) {}

    size_t getLength() const;

    size_t getVisualLength() const;

    bool isEmpty() const;

    int getX() const;
    int getY() const;

    template<TextString T>
    void setText(const T& new_text, bool clear = false, int color_pair = 0) {
        if (clear) needsClear = std::max(needsClear, getVisualLength());
        textChunks = std::vector<TextChunk<T>>({{color_pair, new_text}});
    }

    std::variant<std::string, std::wstring> getText();

    void setX(int px);
    void setY(int py);

    void render();

    void clear();

    void reset();
};

