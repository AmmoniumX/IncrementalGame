#include "Window.hpp"

void Window::WindowDeleter::operator()(WINDOW* win) const {
    if (win) {
        delwin(win);
    }
}

Window::WinUniqPtr Window::newWin(int height, int width, int y, int x) {
    return WinUniqPtr(newwin(height, width, y, x));
}


std::reference_wrapper<Text> Window::setTitle(const std::string& text, Alignment alignment, int color_pair, int offset) {
    if (title) {
        title->setText(text, true, color_pair);
    } else {
        title = &putText(0, 0, text, color_pair);
    }
    switch (alignment) {
        case Alignment::LEFT:
            title->setX(1 + offset);
            break;
        case Alignment::CENTER: {
            int centerX = (width - static_cast<int>(text.size())) / 2 + offset;
            title->setX(centerX);
            break;
        }
        case Alignment::RIGHT: {
            int rightX = width - static_cast<int>(text.size()) - 1 - offset;
            title->setX(rightX);
            break;
        }
    }
    title->setY(0);
    return std::ref(*title);
}

Window::Window(int x, int y, int width, int height, bool visible, int color_pair, WINDOW *parentWin)
: win(newWin(height, width, y, x)), x(x), y(y), width(width), height(height), visible(visible),
  color_pair(color_pair), parentWin(parentWin)
{
    if (color_pair > 0) {
        wbkgd(win.get(), COLOR_PAIR(color_pair));
    }
}

// move constructor for allowing subwindows.emplace_back()
Window::Window(Window&& other) noexcept 
: win(std::move(other.win)), x(other.x), y(other.y), width(other.width), height(other.height), 
  visible(other.visible), color_pair(other.color_pair), parentWin(other.parentWin)
{
    if (color_pair > 0) {
        wbkgd(win.get(), COLOR_PAIR(color_pair));
    }
}

// move assignment operator
Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        win = std::move(other.win);
        x = other.x;
        y = other.y;
        width = other.width;
        height = other.height;
        visible = other.visible;
        color_pair = other.color_pair;
        parentWin = other.parentWin;
    }
    return *this;
}

void Window::setColorPair(int col) {
    color_pair = col;
    wbkgd(win.get(), COLOR_PAIR(color_pair));
}

bool Window::isVisible() const { return visible; }
void Window::clearWindow() {
    // Temporarily set window color to the parent window's color
    if (parentWin) {
        wbkgd(win.get(), getbkgd(parentWin));
    } else {
        wbkgd(win.get(), COLOR_PAIR(GAME_COLORS::DEFAULT)); // Default background
    }
    werase(win.get()); // Clear the window
    wrefresh(win.get()); // Refresh to apply changes
    // Restore the original color
    wbkgd(win.get(), COLOR_PAIR(color_pair));
    // Overwrite the text with spaces
    for (const auto& text : texts) {
        text->clear();
    }
}
void Window::enable() { 
    visible = true; 
}
void Window::disable() { 
    clearWindow();
    visible = false; 
}
void Window::toggle() { visible ? disable() : enable(); }

void Window::render() {
    if (!win) {
        std::println(stderr, "Window is not initialized!");
        return;
    }
    if (!visible) return;

    onTick(); // Call ticker

    box(win.get(), 0, 0); // Draw the border

    // Render the title if it exists
    if (title) {
        title->render();
    }

    // Render the texts
    for (const auto& text : texts) {
        text->render();
    }

    // Render subwindows
    for (const auto& subwindow : subwindows) {
        subwindow->render();
    }

    // Refresh the window
    wrefresh(win.get());
}

Window &Window::createSubwindow(int subY, int subX, int subWidth, int subHeight, 
    bool visible, int color_pair) {
    subwindows.push_back(std::make_unique<Window>(subX, subY, subWidth, subHeight, visible, color_pair, win.get()));
    return *subwindows.back();
}

void Window::onTick() {}
