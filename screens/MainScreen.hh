#pragma once

#include "../core/game.hh"
#include "../core/render.hh"
#include "../resources/Clicker.hh"
#include "../resources/Factory.hh"
#include <sstream>
#include <cmath>

class MainScreen : public Screen {
private:
    static constexpr double NOTIF_DURATION = 1.5 * FRAME_RATE;
    GameDataPtr data;
    TextPtr notifyText;
    double notifyTime = 0;
    TextPtr mainScreenTitle;
    TextPtr mainScreenScore;
    TextPtr mainScreenClickers;
    TextPtr mainScreenFactories;
    TextPtr mainScreenInfo;
    WindowPtr buyWindow;
    TextPtr buyWindowTitle;
    std::vector<TextPtr> buyWindowContent;

    // Local variable copies
    BigNum points;
    BigNum clickers;
    int clicker_lvl;
    double clicker_spc;
    BigNum clicker_cost;
    BigNum factories;
    BigNum factory_cps;
    BigNum factory_cost;

    void notify(const std::string& text) {
        notifyText->setText(text, true);
        notifyTime = NOTIF_DURATION;
    }

    void refreshValues() {
        points = data->getPoints();
        clickers = Clicker::getCount(data);
        clicker_lvl = Clicker::getLevel(data).to_number().value_or(1);
        clicker_spc = Clicker::getSpC(data);
        clicker_cost = Clicker::getCost(data);

        factories = Factory::getCount(data);
        factory_cps = Factory::getCpS(data);
        factory_cost = Factory::getCost(data);
    }

    MainScreen(const GameDataPtr data) : Screen(), data(data) {
        
        // Get initial values
        refreshValues();

        // Create screen elements
        mainScreenTitle = putText(0, 0, "Hello, world!");
        mainScreenScore = putText(1, 0, "Points: " + points.to_string());
        mainScreenClickers = putText(2, 0, "Clickers: " + clickers.to_string() + " (Lvl " + std::to_string(clicker_lvl) + ")");
        mainScreenFactories = putText(3, 0, "Factories: " + factories.to_string());
        mainScreenInfo = putText(4, 0, "[ENTER] Click | [B] Buy | [Q] Quit");
        buyWindow = createWindow(8, 0, COLS-1, 10, false);
        buyWindow->putText(0, 2, "Buy Menu:");
        std::stringstream spc_ss; spc_ss << std::fixed << std::setprecision(1) << clicker_spc;
        buyWindowContent.push_back(buyWindow->putText(1, 1, "[1] Clicker: Clicks once every "+spc_ss.str()+"s. 10 points"));
        buyWindowContent.push_back(buyWindow->putText(2, 1, "[2] LVL Clicker: Speeds up clicker speed by 0.1s. Max 10 levels. 100 points"));
        buyWindowContent.push_back(buyWindow->putText(3, 1, "[3] Factory: Produces 0 clickers every second. 1000 points"));
        buyWindowContent.push_back(buyWindow->putText(7, 1, "[B] Close"));
        notifyText = putText(LINES-1, 0, "");

        setOnTick([this](const GameDataPtr data, const char input) { return this->onTick(data, input); });
    }
public:
    static ScreenPtr create(const GameDataPtr data) {
        return std::shared_ptr<MainScreen>(new MainScreen(data));
    }

    MainScreen() = delete;

    bool onTick(const GameDataPtr data, const char input) {
        // Update local variables
        refreshValues();

        // Update screen elements
        if (notifyTime > 0) { notifyTime--; }
        if (notifyTime == 0) {
            notifyText->reset();
        }
        mainScreenScore->setText("Points: " + points.to_string(), true);
        mainScreenClickers->setText("Clickers: " + clickers.to_string() + " (Lvl " + std::to_string(clicker_lvl) + ")");
        mainScreenFactories->setText("Factories: " + factories.to_string());
        std::stringstream spc_ss; spc_ss << std::fixed << std::setprecision(1) << clicker_spc;
        buyWindowContent[0]->setText("[1] Clicker: Clicks once every "+spc_ss.str()+"s. "+clicker_cost.to_string()+" points");
        buyWindowContent[2]->setText("[3] Factory: Produces "+factory_cps.to_string()+" clickers every second. "+factory_cost.to_string()+" points");

        // Handle input
        switch (input) {
            case 'q':
                return true;
            case '\n':
                data->addPoints(N(1));
                return false;
            case 'b':
                buyWindow->toggle();
                return false;
            case '1':
                if (!buyWindow->isVisible()) return false;
                if (points >= clicker_cost) {
                    data->addResource(Clicker::clicker, N(1));
                    data->subPoints(clicker_cost);
                } else {
                    notify("Not enough points to buy clicker! (Need "+clicker_cost.to_string()+")");
                }
                return false;
            case '2':
                if (!buyWindow->isVisible()) return false;
                if (points < 100) { notify("Not enough points to buy clicker level! (Need 100)"); return false; }
                if (clicker_lvl >= 10) { notify("Max level reached!"); return false; }
                data->addResource(Clicker::clicker_lvl, N(1));
                data->subPoints(N(100));
                return false;
            case '3':
                if (!buyWindow->isVisible()) return false;
                if (points >= factory_cost) {
                    data->addResource(Factory::factory, N(1));
                    data->subPoints(factory_cost);
                } else {
                    notify("Not enough points to buy factory! (Need "+factory_cost.to_string()+")");
                }
                return false;
            case -1:
                return false;
            default:
                notify(std::string("Unknown command: ")+input+" ("+std::to_string(static_cast<int>(input))+")");
                std::cerr << "Unknown command: " << input << " (" << static_cast<int>(input) << ")" << std::endl;
                return false;
        }
    }
};