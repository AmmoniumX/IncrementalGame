#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <curses.h>

class Text {
public:
  struct TextChunk {
    int color_pair{0};
    std::string text{};
  };

private:
  int y, x;

  std::vector<TextChunk> textChunks;

  WINDOW *win = stdscr;
  size_t needsClear = 0;
  bool clearStr = false;

  void doClear();

  static size_t getVisualLengthOf(const TextChunk &chunk);

public:
  Text(int y, int x, const std::string txt, int color_pair = 0,
       WINDOW *win = stdscr)
      : y(y), x(x),
        textChunks(std::vector<TextChunk>({TextChunk{color_pair, txt}})),
        win(win) {}

  Text(int y, int x, const std::span<const TextChunk> chunks,
       WINDOW *win = stdscr)
      : y(y), x(x), textChunks(chunks.begin(), chunks.end()), win(win) {}

  Text(int y, int x, const std::initializer_list<TextChunk> &chunks,
       WINDOW *win = stdscr)
      : y(y), x(x), textChunks(chunks.begin(), chunks.end()), win(win) {}

  size_t getLength() const;

  size_t getVisualLength() const;

  bool isEmpty() const;

  int getX() const;
  int getY() const;

  void setText(std::string new_text, bool clear = false, int color_pair = 0) {
    if (clear)
      needsClear = std::max(needsClear, getVisualLength());
    textChunks = std::vector<TextChunk>({{color_pair, new_text}});
  }

  std::string getText();

  void setX(int px);
  void setY(int py);

  void render();

  void clear();

  void reset();
};
