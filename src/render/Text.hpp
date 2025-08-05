#pragma once

#include <cstddef>
#include <cstdio>
#include <algorithm>
#include <string>
#include <memory>
#include <variant>
#include <vector>
#include <ncursesw/ncurses.h>
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

    std::shared_ptr<WINDOW> win;
    size_t needsClear = 0;
    bool clearStr = false;

    WINDOW* getWin() const {
        return win ? win.get() : stdscr;
    }

    void doClear() {
        if (needsClear <= 0) { return; }
        std::string blank(needsClear, ' ');
        mvwaddstr(getWin(), y, x, blank.c_str());
        if (clearStr) {
            std::visit([&](auto& chunks) {
                for (auto &chunk : chunks) {
                    chunk.text.clear();
                    clearStr = false;
                }
            }, textChunks);
        }
        needsClear = 0;
    }

    static size_t getVisualLengthOf(TextChunk<std::string> chunk) {
        return chunk.text.size();
    }

    static size_t getVisualLengthOf(TextChunk<std::wstring> chunk) {
        return static_cast<size_t>(
            wcswidth(chunk.text.c_str(), chunk.text.size())
        );
    }

public:
    Text(int y, int x, const std::string& txt, int color_pair = 0, std::shared_ptr<WINDOW> win = nullptr)
        : y(y), x(x), textChunks(std::vector<TextChunk<std::string>>({{color_pair, txt}})), win(win) {}

    Text(int y, int x, const std::wstring& txt, int color_pair = 0, std::shared_ptr<WINDOW> win = nullptr)
        : y(y), x(x), textChunks(std::vector<TextChunk<std::wstring>>({{color_pair, txt}})), win(win) {}

    template<TextString T>
    Text(int y, int x, const std::span<const TextChunk<T>>& chunks, std::shared_ptr<WINDOW> win = nullptr)
        : y(y), x(x), textChunks(std::vector<TextChunk<T>>(chunks.begin(), chunks.end())), win(win) {}

    template<TextString T>
    Text(int y, int x, const std::initializer_list<const TextChunk<T>>& chunks, std::shared_ptr<WINDOW> win = nullptr)
        : y(y), x(x), textChunks(std::vector<TextChunk<T>>(chunks.begin(), chunks.end())), win(win) {}

    size_t getLength() const {
        return std::visit([](const auto& chunks) {
            auto view = chunks 
                | std::views::transform([](const auto& chunk) {
                    return chunk.text.size(); });
            return std::accumulate(view.begin(), view.end(), static_cast<size_t>(0));

        }, textChunks);
    }

    size_t getVisualLength() const {
        return std::visit([](const auto& chunks) {
            auto view = chunks 
                | std::views::transform([](const auto& chunk) {
                    return getVisualLengthOf(chunk);
                });
            return std::accumulate(view.begin(), view.end(), static_cast<size_t>(0));
        }, textChunks);
    }

    bool isEmpty() const {
        return std::visit([](const auto& chunks) { 
            return chunks.size() <= 0; 
        }, textChunks);
    }

    int getX() const { return x; }
    int getY() const { return y; }

    void setText(const std::string& new_text, bool clear = false, int color_pair = 0) {
        if (clear) needsClear = std::max(needsClear, getVisualLength());
        textChunks = std::vector<TextChunk<std::string>>({{color_pair, new_text}});
    }

    void setText(const std::wstring& new_text, bool clear = false, int color_pair = 0) {
        if (clear) needsClear = std::max(needsClear, getVisualLength());
        textChunks = std::vector<TextChunk<std::wstring>>({{color_pair, new_text}});
    }

    std::variant<std::string, std::wstring> getText() {
        return std::visit([](const auto& chunks) -> std::variant<std::string, std::wstring> {
            using T = std::decay_t<decltype(chunks)>;
            using T_Str = std::conditional_t<
                std::is_same_v<T, std::vector<TextChunk<std::wstring>>>,
                std::wstring,
                std::string
            >;

            if constexpr (std::is_same_v<T_Str, std::wstring>) {
                std::wstring s;
                for (const auto& chunk : chunks) {
                    s.append(chunk.text);
                }
                return s;
            } else {
                std::string s;
                for (const auto& chunk : chunks) {
                    s.append(chunk.text);
                }
                return s;
            }

        }, textChunks);
    }

    void setX(int px) { x = px; }
    void setY(int py) { y = py; }

    void render() {
        doClear();
        if (isEmpty()) return;

        WINDOW* w = getWin();
        std::visit([&](const auto& chunks) {
            using T = std::decay_t<decltype(chunks)>;
            using T_Str = std::conditional_t<
                std::is_same_v<T, std::vector<TextChunk<std::wstring>>>,
                std::wstring,
                std::string
            >;

            size_t dX = 0;
            for (const auto& chunk : chunks) {
                if (chunk.color_pair > 0) {
                    wattron(w, COLOR_PAIR(chunk.color_pair));
                }
                if constexpr (std::is_same_v<T_Str, std::wstring>) {
                    mvwaddwstr(w, y, x+dX, chunk.text.c_str());
                } else {
                    mvwaddstr(w, y, x+dX, chunk.text.c_str());
                }

                if (chunk.color_pair > 0) {
                    wattroff(w, COLOR_PAIR(chunk.color_pair));
                }

                dX += getVisualLengthOf(chunk);
            }

        }, textChunks);

    }

    void clear() {
        needsClear = std::max(needsClear, getVisualLength());
    }

    void reset() {
        clear();
        clearStr = true;
    }
};

using TextPtr = std::shared_ptr<Text>;
