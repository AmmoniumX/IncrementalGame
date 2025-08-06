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
#include <functional>
#include <iostream>
#include <map>
#include <print>
#include <unordered_map>
#include <variant>

using namespace std::string_literals;

class MainScreen : public Screen {
  private:
    static constexpr int NOTIF_DURATION = static_cast<int>(1.5 * FRAME_RATE);
    std::reference_wrapper<Text> notifyText;
    int notifyTime = 0;
    bool notifyClear = false;

    std::reference_wrapper<Window> inventoryWindow;
    std::array<std::reference_wrapper<Text>, 3> inventoryContents;
    ResourcePtr inventory;

    std::reference_wrapper<Window> craftingWindow;
    std::map<std::string, std::reference_wrapper<Text>> craftingOptions;

    std::reference_wrapper<Window> upgradesWindow;
    std::map<std::string, std::reference_wrapper<Text>> upgradeOptions;

    std::reference_wrapper<Window> sidebarCraftingWindow;
    std::reference_wrapper<Window> sidebarUpgradesWindow;

    enum Subwindows { 
        CRAFTING = 0, 
        UPGRADES = 1
    };
    struct WindowGroup {
        std::reference_wrapper<Window> main;
        std::reference_wrapper<Window> sidebar;
        int activeColor;
        int inactiveColor;
        WindowGroup() = delete; // delete default constructor: invalid with std::reference_wrapper
        WindowGroup(Window& m, Window& s, int a, int i)
        : main(m), sidebar(s), activeColor(a), inactiveColor(i) {}
    };
    
    std::unordered_map<Subwindows, WindowGroup> windows;

    Subwindows activeWindow = CRAFTING;

    void notify(const std::string &text) {
        notifyText.get().setText(text, true);
        notifyTime = NOTIF_DURATION;
        notifyClear = false;
    }

    void rotateWindows() {
        WindowGroup w = windows.at(activeWindow);
        (w.main.get()).disable();
        (w.sidebar.get()).setColorPair(w.inactiveColor);
        activeWindow = Subwindows((activeWindow + 1) % windows.size());
        w = windows.at(activeWindow);
        (w.main.get()).enable();
        (w.sidebar.get()).setColorPair(w.activeColor);
    }

    void switchWindow(Subwindows target) {
        WindowGroup w = windows.at(activeWindow);
        if (target == activeWindow) { return; }
        (w.main.get()).disable();
        (w.sidebar.get()).setColorPair(w.inactiveColor);
        activeWindow = target;
        w = windows.at(activeWindow);
        (w.main.get()).enable();
        (w.sidebar.get()).setColorPair(w.activeColor);
    }

    void refreshInventoryCounts() {
        if (!inventory) { return; }

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
            if (inventoryContents[currLine].get().getVisualLength() + entrySize + 1 <
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
            auto oldText = inventoryContents[i].get().getText();
            auto oldTextStr = std::get_if<std::string>(&oldText);
            if (oldTextStr) {
                if (*oldTextStr == display_lines[i]) { return; }
            }
            inventoryContents[i].get().setText(display_lines[i], true);
        }
    }

  public:
    virtual ~MainScreen() override = default;

    MainScreen() :
        Screen(),
        // Initialize reference_wrapper members here
        notifyText(putText(LINES - 1, 0, "")),
        inventoryWindow(createWindow(0, 0, COLS, 5, true, GAME_COLORS::GRAY_BLACK)),
        inventoryContents({
            inventoryWindow.get().putText(1, 2, ""s, GAME_COLORS::WHITE_BLACK),
            inventoryWindow.get().putText(2, 2, ""s, GAME_COLORS::WHITE_BLACK),
            inventoryWindow.get().putText(3, 2, ""s, GAME_COLORS::WHITE_BLACK)
        }),
        inventory(ResourceManager.getResource(Inventory::RESOURCE_ID)),
        craftingWindow(createWindow(5, 12, COLS - 12, LINES - 6, true, GAME_COLORS::YELLOW_BLACK)),
        upgradesWindow(createWindow(5, 12, COLS - 12, LINES - 6, false, GAME_COLORS::RED_BLACK)),
        sidebarCraftingWindow(createWindow(5, 0, 12, 3, true, GAME_COLORS::YELLOW_GRAY)),
        sidebarUpgradesWindow(createWindow(8, 0, 12, 3, true, GAME_COLORS::RED_BLACK)),
        windows({
            {CRAFTING, WindowGroup(craftingWindow, sidebarCraftingWindow, GAME_COLORS::YELLOW_GRAY, GAME_COLORS::YELLOW_BLACK)},
            {UPGRADES, WindowGroup(upgradesWindow, sidebarUpgradesWindow, GAME_COLORS::RED_GRAY, GAME_COLORS::RED_BLACK)}
        })
    {
        // The rest of your constructor body
        (void)inventoryWindow.get().setTitle("Inventory", Window::Alignment::CENTER, GAME_COLORS::YELLOW_BLACK);
        (void)upgradesWindow.get().setTitle("Upgrades", Window::Alignment::LEFT, GAME_COLORS::RED_BLACK, 1);
        upgradeOptions.emplace("example_upgrade", upgradesWindow.get().putText(1, 1, "Example"s));
        (void)craftingWindow.get().setTitle("Crafting", Window::Alignment::LEFT, GAME_COLORS::YELLOW_BLACK, 1);
        craftingOptions.emplace(Inventory::Items::IRON, craftingWindow.get().putText(1, 1, "[1] Iron Ingots"s));
        craftingOptions.emplace(Inventory::Items::COPPER, craftingWindow.get().putText(2, 1, "[2] Copper Ingots"s));
        craftingOptions.emplace(Inventory::Items::IRON_GEAR, craftingWindow.get().putText<std::string>(3, 1, {
                                    {GAME_COLORS::YELLOW_BLACK, "[3] Iron Gear "s},
                                    {GAME_COLORS::GRAY_BLACK, "(requires: 2 Iron Ingot)"s}
                                        }));
        (void)sidebarCraftingWindow.get().putText(1, 1, "[C]rafting"s, GAME_COLORS::DEFAULT);
        (void)sidebarUpgradesWindow.get().putText(1, 1, "[U]pgrades"s, GAME_COLORS::DEFAULT);
    }

    static std::unique_ptr<Screen> create() { return std::make_unique<MainScreen>(); }

    bool onTick(const char input) override {

        // Update screen elements
        if (notifyTime > 0) {
            notifyTime--;
        }
        if (notifyTime <= 0 && !notifyClear) {
            notifyText.get().reset();
            notifyClear = true;
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
