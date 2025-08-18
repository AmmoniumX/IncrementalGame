#include <ranges>
#include <string>
#include <type_traits>
#include <variant>
#include <numeric>
#include <wchar.h>
#include <uchar.h>
#include <curses.h>

#include "./wutils.hpp"

#include "Text.hpp"

void Text::doClear() {
    if (needsClear <= 0) { return; }
    std::string blank(needsClear, ' ');
    mvwaddstr(win, y, x, blank.c_str());
    if (clearStr) {
        std::visit([](auto& chunks) {
            for (auto &chunk : chunks) {
                chunk.text.clear();
            }
        }, textChunks);
        clearStr = false;
    }
    needsClear = 0;
}

size_t Text::getVisualLengthOf(TextChunk<std::string> chunk) {
    return chunk.text.size();
}

size_t Text::getVisualLengthOf(TextChunk<std::wstring> chunk) {
    return wutils::wswidth(chunk.text);
}

size_t Text::getLength() const {
    return std::visit([](const auto& chunks) {
        auto view = chunks 
            | std::views::transform([](const auto& chunk) {
                return chunk.text.size(); });
        return std::accumulate(view.begin(), view.end(), static_cast<size_t>(0));

    }, textChunks);
}

size_t Text::getVisualLength() const {
    return std::visit([](const auto& chunks) {
        auto view = chunks 
            | std::views::transform([](const auto& chunk) {
                return getVisualLengthOf(chunk);
            });
        return std::accumulate(view.begin(), view.end(), static_cast<size_t>(0));
    }, textChunks);
}

bool Text::isEmpty() const {
    return std::visit([](const auto& chunks) { 
        return chunks.size() <= 0; 
    }, textChunks);
}

int Text::getX() const { return x; }
int Text::getY() const { return y; }

std::variant<std::string, std::wstring> Text::getText() {
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

void Text::setX(int px) { x = px; }
void Text::setY(int py) { y = py; }

void Text::render() {
    doClear();
    if (isEmpty()) return;

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
                wattron(win, COLOR_PAIR(chunk.color_pair));
            }
            if constexpr (std::is_same_v<T_Str, std::wstring>) {
                mvwaddwstr(win, y, x+dX, chunk.text.c_str());
            } else {
                mvwaddstr(win, y, x+dX, chunk.text.c_str());
            }

            if (chunk.color_pair > 0) {
                wattroff(win, COLOR_PAIR(chunk.color_pair));
            }

            dX += getVisualLengthOf(chunk);
        }

    }, textChunks);

}

void Text::clear() {
    needsClear = std::max(needsClear, getVisualLength());
}

void Text::reset() {
    clear();
    clearStr = true;
}
