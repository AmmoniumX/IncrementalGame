#pragma once

#include "../headers/game.hh"
#include "../headers/render.hh"

class MainScreen : public Screen {
private:
    GameDataPtr data;
    TextPtr mainScreenTitle;
    TextPtr mainScreenScore;
    TextPtr mainScreenInfo;
    WindowPtr buyWindow;

    MainScreen(const GameDataPtr data) : Screen(), data(data) {
        mainScreenTitle = putText(0, 0, "Hello, world!");
        mainScreenScore = putText(1, 0, "Points: " + data->points.to_string());
        mainScreenInfo = putText(2, 0, "Press 'ENTER' to earn points, 'b' to buy, 'q' to quit");
        buyWindow = createWindow(16, 0, 20, 10, false);
        buyWindow->putText(1, 1, "Buy stuff here");

        setOnTick([this](const GameDataPtr data, const char input) { return this->onTick(data, input); });
    }
public:
    static ScreenPtr create(const GameDataPtr data) {
        return std::shared_ptr<MainScreen>(new MainScreen(data));
    }

    bool onTick(const GameDataPtr data, const char input) {
        mainScreenScore->setText("Points: " + data->points.to_string());
        switch (input) {
            case 'q':
                return true;
            case '\n':
                data->points++;
                return false;
            case 'b':
                buyWindow->toggle();
                return false;
            case -1:
                return false;
            default:
                mvprintw(LINES-1, 0, "Unknown command: %c (%d)", input, (int)input);
                std::cerr << "Unknown command: " << input << " (" << (int)input << ")" << std::endl;
                return false;
        }
    }
};