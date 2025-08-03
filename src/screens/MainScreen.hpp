#pragma once

#include "../ResourceManager.hpp"
#include "../game.hpp"
#include "../render/Screen.hpp"
#include "../render/Text.hpp"
#include "../render/Window.hpp"
#include "../resources/Inventory.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <iostream>
#include <map>
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
    std::map<std::string, TextPtr> craftingOptions;

    WindowPtr upgradesWindow;
    std::map<std::string, TextPtr> upgradeOptions;

    WindowPtr sidebarUpgradesWindow;
    WindowPtr sidebarCraftingWindow;

    enum Subwindows { 
        CRAFTING = 0, 
        UPGRADES = 1
    };
    std::array<WindowPtr*, 2> windowOrder = {&craftingWindow, &upgradesWindow};
    std::map<Subwindows, WindowPtr*> sidebarWindow = {
        {CRAFTING, &sidebarCraftingWindow},
        {UPGRADES, &sidebarUpgradesWindow}
    };
    std::map<Subwindows, int> inactiveColor = {
        {CRAFTING, GAME_COLORS::YELLOW_BLACK},
        {UPGRADES, GAME_COLORS::RED_BLACK}
    };
    std::map<Subwindows, int> activeColor = {
    {CRAFTING, GAME_COLORS::YELLOW_GRAY},
    {UPGRADES, GAME_COLORS::RED_GRAY}
    };
    Subwindows activeWindow = CRAFTING;
    size_t expectedSize = 2;

    void notify(const std::string &text) {
        notifyText->setText(text, true);
        notifyTime = NOTIF_DURATION;
    }

    void rotateWindows() {
        (*windowOrder[activeWindow])->disable();
        (*sidebarWindow[activeWindow])->setColorPair(inactiveColor[activeWindow]);
        activeWindow = Subwindows((activeWindow + 1) % windowOrder.size());
        (*windowOrder[activeWindow])->enable();
        (*sidebarWindow[activeWindow])->setColorPair(activeColor[activeWindow]);
    }

    void switchWindow(Subwindows target) {
        if (target == activeWindow) { return; }
        (*windowOrder[activeWindow])->disable();
        (*sidebarWindow[activeWindow])->setColorPair(inactiveColor[activeWindow]);
        activeWindow = target;
        (*windowOrder[activeWindow])->enable();
        (*sidebarWindow[activeWindow])->setColorPair(activeColor[activeWindow]);
    }

    void refreshInventoryCounts() {
        if (!inventory)
            return;

        auto lockedInventory = inventory->synchronize();
        Inventory *inv = static_cast<Inventory *>(*lockedInventory);

        const std::map<std::string, BigNum> items = inv->getItems();

        static size_t charsPerLine = COLS - 2;
        std::array<std::string, 3> display_lines({"", "", ""});
        int currLine = 0;
        for (const auto &[item, num] : items) {
            std::string count = num.to_pretty_string();
            std::string entry = std::format("{}: {}", item, count);
            size_t entrySize = entry.size();

            // Set entry to first line that has enough space
            if (inventoryContents[currLine]->getVisualLength() + entrySize + 1 <
                charsPerLine) {
                if (currLine < 3) {
                    display_lines[currLine] += entry + " ";
                } else {
                    std::println(
                        std::cerr,
                        "Inventory overflow, cannot display all items.");
                    break;
                }
            } else {
                // Move to next line if current line is full
                currLine++;
                if (currLine < 3) {
                    display_lines[currLine] = entry + " ";
                } else {
                    std::println(
                        std::cerr,
                        "Inventory overflow, cannot display all items.");
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
    virtual ~MainScreen() = default;

    MainScreen() : Screen() {
        assert(
            windowOrder.size() == expectedSize &&
            sidebarWindow.size() == expectedSize &&
            inactiveColor.size() == expectedSize &&
            activeColor.size() == expectedSize &&
            "Active window variables have non-matching sizes!"
        );
        // Get inventory
        inventory = ResourceManager.getResource(Inventory::RESOURCE_ID);

        // Create screen elements
        inventoryWindow =
            createWindow(0, 0, COLS, 5, true, GAME_COLORS::GRAY_BLACK);
        (void)inventoryWindow->setTitle("Inventory", Window::Alignment::CENTER,
                                        GAME_COLORS::YELLOW_BLACK);
        for (int i = 0; i < 3; ++i) {
            inventoryContents[i] = inventoryWindow->putText(
                i + 1, 2, "", GAME_COLORS::WHITE_BLACK);
        }

        upgradesWindow = createWindow(5, 12, COLS - 12, LINES - 6, false,
                                      GAME_COLORS::RED_BLACK);
        (void)upgradesWindow->setTitle("Upgrades", Window::Alignment::LEFT,
                                       GAME_COLORS::RED_BLACK, 1);
        upgradeOptions.emplace("example_upgrade",
                               upgradesWindow->putText(1, 1, "Example"));

        craftingWindow = createWindow(5, 12, COLS - 12, LINES - 6, true,
                                      GAME_COLORS::YELLOW_BLACK);
        (void)craftingWindow->setTitle("Crafting", Window::Alignment::LEFT,
                                       GAME_COLORS::YELLOW_BLACK, 1);
        craftingOptions.emplace(Inventory::Items::IRON,
                                craftingWindow->putText(1, 1, "[1] Iron Ingots"));
        craftingOptions.emplace(
            Inventory::Items::COPPER,
            craftingWindow->putText(2, 1, "[2] Copper Ingots"));

        activeWindow = CRAFTING;

        sidebarCraftingWindow =
            createWindow(5, 0, 12, 3, true, GAME_COLORS::YELLOW_GRAY);
        (void)sidebarCraftingWindow->putText(1, 1, "[C]rafting",
                                             GAME_COLORS::DEFAULT);
        sidebarUpgradesWindow =
            createWindow(8, 0, 12, 3, true, GAME_COLORS::RED_BLACK);
        (void)sidebarUpgradesWindow->putText(1, 1, "[U]pgrades",
                                             GAME_COLORS::DEFAULT);

        // Create notification text
        notifyText = putText(LINES - 1, 0, "");
    }

    static ScreenPtr create() { return std::make_shared<MainScreen>(); }

    bool onTick(const char input) override {

        // Update screen elements
        if (notifyTime > 0) {
            notifyTime--;
        }
        if (notifyTime == 0) {
            notifyText->reset();
        }
        refreshInventoryCounts();

        // Handle input
        auto lockedInventory = inventory->synchronize();
        Inventory *inv = static_cast<Inventory *>(*lockedInventory);
        switch (input) {
        case 'q':
            return true;
        case '1':
            inv->addItem(Inventory::Items::IRON, N(1));
            return false;
        case '2':
            inv->addItem(Inventory::Items::COPPER, N(1));
            return false;
        case 'C':
            switchWindow(CRAFTING);
            return false;
        case 'U':
            switchWindow(UPGRADES);
            return false;
        case '\t':
            rotateWindows();
            return false;
        case -1:
            return false;
        default:
            notify(std::string("Unknown command: ") + input + " (" +
                   std::to_string(static_cast<int>(input)) + ")");
            return false;
        }
    }
};
