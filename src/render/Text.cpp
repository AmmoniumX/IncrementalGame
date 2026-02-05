#include <curses.h>
#include <numeric>
#include <ranges>
#include <string>
#include <uchar.h>
#include <wchar.h>

#include "Text.hpp"

void Text::doClear() {
  if (needsClear <= 0) {
    return;
  }
  std::string blank(needsClear, ' ');
  mvwaddstr(win, y, x, blank.c_str());
  if (clearStr) {
    for (auto &chunk : textChunks) {
      chunk.text.clear();
    }
    clearStr = false;
  }
  needsClear = 0;
}

size_t Text::getVisualLengthOf(const TextChunk &chunk) {
  return chunk.text.length();
}

size_t Text::getLength() const {
  auto view = textChunks | std::views::transform([](const auto &chunk) {
                return chunk.text.size();
              });
  return std::reduce(view.begin(), view.end(), 0z);
}

size_t Text::getVisualLength() const {
  auto view = textChunks | std::views::transform([](const auto &chunk) {
                return getVisualLengthOf(chunk);
              });
  return std::reduce(view.begin(), view.end(), 0z);
}

bool Text::isEmpty() const { return textChunks.size() <= 0; }

int Text::getX() const { return x; }
int Text::getY() const { return y; }

std::string Text::getText() {
  std::string s;
  for (const auto &chunk : textChunks) {
    s.append(chunk.text);
  }
  return s;
}

void Text::setX(int px) { x = px; }
void Text::setY(int py) { y = py; }

void Text::render() {
  doClear();
  if (isEmpty())
    return;

  size_t dX = 0;
  for (const auto &chunk : textChunks) {
    if (chunk.color_pair > 0) {
      wattron(win, COLOR_PAIR(chunk.color_pair));
    }
    mvwaddstr(win, y, x + dX, chunk.text.c_str());

    if (chunk.color_pair > 0) {
      wattroff(win, COLOR_PAIR(chunk.color_pair));
    }

    dX += getVisualLengthOf(chunk);
  }
}

void Text::clear() { needsClear = std::max(needsClear, getVisualLength()); }

void Text::reset() {
  clear();
  clearStr = true;
}
