#pragma once

#include <string>
#include <print>
#include <vector>
#include <memory>

#include "./Text.hpp"
#include "./Window.hpp"

/*
* @class Screen
* @brief A class to represent and render a screen with multiple windows and texts.
*/
class Screen {
private:
    std::vector<std::unique_ptr<Text>> texts; // Screen-level texts
    std::vector<std::unique_ptr<Window>> windows; // Screen-level windows

public:
    virtual ~Screen() = default;

    Text &putText(int y, int x, const std::string& text, int color_pair=0);

    Window &createWindow(int y, int x, int width, int height, bool visible=true, int color_pair=0);

    void render();

    virtual void onTick();
};

