#pragma once

#include "../game.hpp"
#include "../resourceRegistry.hpp"
#include "../render/Text.hpp"
#include "../render/Window.hpp"
#include "../render/Screen.hpp"
#include "../resources/Inventory.hpp"
#include <sstream>
#include <cmath>
#include <map>
#include <array>
#include <vector>
#include <iostream>
#include <format>
#include <print>

class MainScreen : public Screen {
private:
    static constexpr double NOTIF_DURATION = 1.5 * FRAME_RATE;
    TextPtr notifyText;
    double notifyTime = 0;

    WindowPtr inventoryWindow;
    std::array<TextPtr, 3> inventoryContents;
    ResourcePtr inventory;

    WindowPtr craftingWindow;
    std::vector<TextPtr> craftingOptions;

    void notify(const std::string& text) {
        notifyText->setText(text, true);
        notifyTime = NOTIF_DURATION;
    }

    void refreshInventoryCounts() {
        if (!inventory) return;
        
        auto lockedInventory = inventory->synchronize();
        Inventory *inv = static_cast<Inventory*>(*lockedInventory);

        const std::map<std::string, BigNum> items = inv->getItems();
        
        static size_t charsPerLine = COLS - 2;
        std::array<std::string, 3> display_lines({"", "", ""});
        int currLine = 0;
        for (const auto& [item, num] : items) {
            std::string count = num.to_pretty_string();
            std::string entry = std::format("{}: {}", item, count);
            size_t entrySize = entry.size();

            // Set entry to first line that has enough space
            if (inventoryContents[currLine]->getText().length() + entrySize + 1 < charsPerLine) {
                if (currLine < 3) {
                    display_lines[currLine] += entry + " ";
                } else {
                    std::println(std::cerr, "Inventory overflow, cannot display all items.");
                    break;
                }
            } else {
                // Move to next line if current line is full
                currLine++;
                if (currLine < 3) {
                    display_lines[currLine] = entry + " ";
                } else {
                    std::println(std::cerr, "Inventory overflow, cannot display all items.");
                    break;
                }
            }
        }

        // Set display lines
        for (int i = 0; i < 3; i++) {
            inventoryContents[i]->setText(display_lines[i], true);
        }
        
    }

public:
    MainScreen() : Screen() {
        // Get inventory
        inventory = ResourceRegistry.getResource(Inventory::RESOURCE_ID);

        // Create screen elements
        inventoryWindow = createWindow(0, 0, COLS, 5, true, GAME_COLORS::GRAY_BLACK);
        (void) inventoryWindow->setTitle("Inventory", Window::Alignment::CENTER, GAME_COLORS::YELLOW_BLACK);
        for (int i = 0; i < 3; ++i) {
            inventoryContents[i] = inventoryWindow->putText(i + 1, 2, "", GAME_COLORS::WHITE_BLACK);
        }

        craftingWindow = createWindow(5, 0, COLS, LINES-6, true, GAME_COLORS::YELLOW_BLACK);
        (void) craftingWindow->setTitle("Crafting", Window::Alignment::LEFT, GAME_COLORS::YELLOW_BLACK);
        craftingOptions.emplace_back(craftingWindow->putText(1, 1, "[I]ron Ingots"));
        craftingOptions.emplace_back(craftingWindow->putText(2, 1, "[C]opper Ingots"));

        // Create notification text
        notifyText = putText(LINES-1, 0, "");
    }

    static ScreenPtr create() {
        return std::make_shared<MainScreen>();
    }

    bool onTick(const char input) override {

        // Update screen elements
        if (notifyTime > 0) { notifyTime--; }
        if (notifyTime == 0) {
            notifyText->reset();
        }
        refreshInventoryCounts();

        // Handle input
        auto lockedInventory = inventory->synchronize();
        Inventory *inv = static_cast<Inventory*>(*lockedInventory);
        switch (input) {
            case 'q':
                return true;
            case 'i':
                inv->addItem(Inventory::Items::IRON, N(1));
                return false;
            case 'c':
                inv->addItem(Inventory::Items::COPPER, N(1));
                return false;
            case -1:
                return false;
            default:
                notify(std::string("Unknown command: ")+input+" ("+std::to_string(static_cast<int>(input))+")");
                return false;
        }
    }
};
