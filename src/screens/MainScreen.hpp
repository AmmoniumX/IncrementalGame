#pragma once

#include "../game.hpp"
#include "../resourceRegistry.hpp"
#include "../render/Text.hpp"
#include "../render/Window.hpp"
#include "../render/Screen.hpp"
#include "../resources/Points.hpp"
#include "../resources/Clicker.hpp"
#include "../resources/Factory.hpp"
#include <sstream>
#include <cmath>

class MainScreen : public Screen {
private:
    static constexpr double NOTIF_DURATION = 1.5 * FRAME_RATE;
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
    ResourcePtr pointsResource;
    ResourcePtr clickerResource;
    ResourcePtr factoryResource;
    BigNum points;
    BigNum clickers;
    BigNum speed;
    BigNum prod;
    double clicker_spc;
    BigNum clicker_cost;
    BigNum clicker_speed_cost;
    BigNum clicker_prod_cost;
    BigNum factories;
    BigNum factory_cost;

    void notify(const std::string& text) {
        notifyText->setText(text, true);
        notifyTime = NOTIF_DURATION;
    }

    void refreshValues() {
        points = std::dynamic_pointer_cast<Points>(*(pointsResource->synchronize()))->getPoints();
        auto clickerLocked = clickerResource->synchronize();
        std::shared_ptr<Clicker> clicker = std::dynamic_pointer_cast<Clicker>(*clickerLocked);
        auto factoryLocked = factoryResource->synchronize();
        std::shared_ptr<Factory> factory = std::dynamic_pointer_cast<Factory>(*factoryLocked);
        clickers = clicker->getCount();
        speed = clicker->getSpeed();
        prod = clicker->getProd();
        clicker_spc = clicker->getSpC();
        factories = factory->getCount();
    }

public:
    MainScreen() : Screen() {

        // Get resources
        pointsResource = ResourceRegistry.getResource(Points::RESOURCE_ID);
        clickerResource = ResourceRegistry.getResource(Clicker::RESOURCE_ID);
        factoryResource = ResourceRegistry.getResource(Factory::RESOURCE_ID);
        
        // Get initial values
        refreshValues();

        // Create screen elements
        mainScreenTitle = putText(0, 0, "Hello, world!");
        mainScreenScore = putText(1, 0, "Points: " + points.to_string());
        mainScreenClickers = putText(2, 0, "Clickers: " + clickers.to_string() + " (Speed: " + speed.to_string() + ", Prod:" + prod.to_string() + ")");
        mainScreenFactories = putText(3, 0, "Factories: " + factories.to_string());
        mainScreenInfo = putText(4, 0, "[ENTER] Click | [B] Buy | [Q] Quit");

        // Create buy window
        buyWindow = createWindow(8, 0, COLS-1, 9, false, GAME_COLORS::YELLOW_BLACK);
        buyWindow->putText(0, 2, "Buy Menu:");
        std::stringstream spc_ss; spc_ss << std::fixed << std::setprecision(1) << clicker_spc;
        buyWindowContent.push_back(buyWindow->putText(1, 1, "[1] Clicker: Gains "+prod.to_string()+" points every "+spc_ss.str()+"s. 10 points"));
        buyWindowContent.push_back(buyWindow->putText(2, 1, "[2] Clicker Speed: Speeds up clicker speed by 0.1s. Max 10 levels. 100 points"));
        buyWindowContent.push_back(buyWindow->putText(3, 1, "[3] Clicker Productivity: Increases points per click by 1 (currently 1). 1000 points"));
        buyWindowContent.push_back(buyWindow->putText(4, 1, "[4] Factory: Produces 1 clicker every second. 1000 points"));
        buyWindowContent.push_back(buyWindow->putText(7, 1, "[B] Close", GAME_COLORS::GRAY_BLACK));

        // Create notification text
        notifyText = putText(LINES-1, 0, "");
    }

    static ScreenPtr create() {
        return std::make_shared<MainScreen>();
    }

    bool onTick(const char input) override {
        // Update local variables
        refreshValues();

        // Update screen elements
        if (notifyTime > 0) { notifyTime--; }
        if (notifyTime == 0) {
            notifyText->reset();
        }
        mainScreenScore->setText("Points: " + points.to_string(), true);
        mainScreenClickers->setText("Clickers: " + clickers.to_string() + " (Speed: " + speed.to_string() + ", Prod:" + prod.to_string() + ")");
        mainScreenFactories->setText("Factories: " + factories.to_string());
        std::stringstream spc_ss; spc_ss << std::fixed << std::setprecision(1) << clicker_spc;

        {
            auto clickerLocked = clickerResource->synchronize();
            std::shared_ptr<Clicker> clicker = std::dynamic_pointer_cast<Clicker>(*clickerLocked);
            auto factoryLocked = factoryResource->synchronize();
            std::shared_ptr<Factory> factory = std::dynamic_pointer_cast<Factory>(*factoryLocked);
            clicker_cost = clicker->getCost();
            buyWindowContent[0]->setText("[1] Clicker: Gains "+prod.to_string()+" points every "+spc_ss.str()+"s. "+clicker_cost.to_string()+" points");
            clicker_speed_cost = clicker->getSpeedCost();
            buyWindowContent[1]->setText("[2] Clicker Speed: Speeds up clicker speed by 0.1s. Max 10 levels. "+clicker_speed_cost.to_string()+" points");
            clicker_prod_cost = clicker->getProdCost();
            buyWindowContent[2]->setText("[3] Clicker Productivity: Increases points per click by 1 (currently "+prod.to_string()+"). "+clicker_prod_cost.to_string()+" points");
            factory_cost = factory->getCost();
            buyWindowContent[3]->setText("[4] Factory: Produces 1 clicker every second. "+factory_cost.to_string()+" points");
        }

        // Handle input
        switch (input) {
            case 'q':
                return true;
            case '\n':
                (std::dynamic_pointer_cast<Points>(*(pointsResource->synchronize())))->addPoints(N(1));
                return false;
            case 'b':
                buyWindow->toggle();
                return false;
            case '1':
                if (!buyWindow->isVisible()) return false;
                {
                    auto pointsLocked = pointsResource->synchronize();
                    std::shared_ptr<Points> p = std::dynamic_pointer_cast<Points>(*pointsLocked);
                    auto clickerLocked = clickerResource->synchronize();
                    std::shared_ptr<Clicker> clicker = std::dynamic_pointer_cast<Clicker>(*clickerLocked);

                    points = p->getPoints();
                    clicker_cost = clicker->getCost();
                    if (points >= clicker_cost) {
                        clicker->addCount(N(1));
                        points -= clicker_cost;
                        p->setPoints(points);
                    } else {
                        notify("Not enough points to buy clicker! (Need "+clicker_cost.to_string()+")");
                    }
                }
                return false;
            case '2':
                if (!buyWindow->isVisible()) return false;
                {
                    auto pointsLocked = pointsResource->synchronize();
                    std::shared_ptr<Points> p = std::dynamic_pointer_cast<Points>(*pointsLocked);
                    auto clickerLocked = clickerResource->synchronize();
                    std::shared_ptr<Clicker> clicker = std::dynamic_pointer_cast<Clicker>(*clickerLocked);

                    points = p->getPoints();
                    clicker_speed_cost = clicker->getSpeedCost();
                    if (points < clicker_speed_cost) { notify("Not enough points to buy clicker speed! (Need 100)"); }
                    if (speed >= Clicker::MAX_SPEED) { notify("Max speed reached!"); }
                    clicker->addSpeed(N(1));
                    points -= clicker_speed_cost;
                    p->setPoints(points);
                }
                return false;
            case '3':
                if (!buyWindow->isVisible()) return false;
                {
                    auto pointsLocked = pointsResource->synchronize();
                    std::shared_ptr<Points> p = std::dynamic_pointer_cast<Points>(*pointsLocked);
                    auto clickerLocked = clickerResource->synchronize();
                    std::shared_ptr<Clicker> clicker = std::dynamic_pointer_cast<Clicker>(*clickerLocked);

                    points = p->getPoints();
                    clicker_prod_cost = clicker->getProdCost();
                    if (points < 1000) { notify("Not enough points to buy clicker productivity! (Need 1000)"); return false; }
                    clicker->addProd(N(1));
                    points -= clicker_prod_cost;
                    p->setPoints(points);
                }
                return false;
            case '4':
                if (!buyWindow->isVisible()) return false;
                {
                    auto pointsLocked = pointsResource->synchronize();
                    std::shared_ptr<Points> p = std::dynamic_pointer_cast<Points>(*pointsLocked);
                    auto factoryLocked = factoryResource->synchronize();
                    std::shared_ptr<Factory> factory = std::dynamic_pointer_cast<Factory>(*factoryLocked);

                    points = p->getPoints();
                    if (points >= factory_cost) {
                        factory->addCount(N(1));
                        points -= factory_cost;
                        p->setPoints(points);
                    } else {
                        notify("Not enough points to buy factory! (Need "+factory_cost.to_string()+")");
                    }
                }
                return false;
            case -1:
                return false;
            default:
                notify(std::string("Unknown command: ")+input+" ("+std::to_string(static_cast<int>(input))+")");
                return false;
        }
    }
};
