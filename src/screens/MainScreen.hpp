#pragma once

#include "../render/Screen.hpp"
#include "../render/Text.hpp"
#include "../render/Window.hpp"
#include "../resources/Inventory.hpp"
#include "../resources/Recipes.hpp"
#include "../systems/ScreenManager.hpp"
#include "../systems/ResourceManager.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <functional>
#include <map>
#include <print>
#include <unordered_map>
#include <variant>

using namespace std::string_literals;

class MainScreen : public Screen {
  private:
    static constexpr int NOTIF_DURATION = static_cast<int>(1.5 * FRAME_RATE);
    Text &notifyText;
    int notifyTime = 0;
    bool notifyClear = false;

    Window &inventoryWindow;
    std::array<std::reference_wrapper<Text>, 3> inventoryContents;

    Window &craftingWindow;

    Window &upgradesWindow;
    std::map<std::string, std::reference_wrapper<Text>> upgradeOptions;

    Window &sidebarCraftingWindow;
    Window &sidebarUpgradesWindow;

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

    std::shared_ptr<Resource> inventory;
    std::shared_ptr<Resource> recipes;

    Subwindows activeWindow = CRAFTING;

    void notify(const std::string &text) {
        notifyText.setText(text, true);
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

    template <typename T>
    T getOrThrow(const std::optional<T>& opt, const std::string& msg) {
        if (!opt) throw std::runtime_error(msg);
        return *opt;
    }

    void refreshInventoryCounts() {
        auto syncedInv = inventory->synchronize();
        Inventory *inv = static_cast<Inventory *>(syncedInv->get());

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
                        stderr,
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
                        stderr,
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
            inventoryContents[i].get().setText(display_lines[i], true, GAME_COLORS::WHITE_BLACK);
        }
    }

    std::unordered_map<char, std::function<void(MainScreen*, Inventory*)>> inputListeners;

    void registerListener(char input, std::function<void(MainScreen*, Inventory*)> listener) {
        inputListeners.insert_or_assign(input, listener);
    }

    int numCraftingOptions = 0;
    template<TextString T>
    void addCraftingOption(char input, std::initializer_list<Text::TextChunk<T>> init, 
            Recipes::Recipe recipe) {
        craftingWindow.putText<std::string>(++numCraftingOptions, 1, init);
        registerListener(input, [recipe](MainScreen *scr, Inventory *inv) {
            scr->attemptRecipe(inv, recipe);
        });
    }

    bool attemptRecipe(Inventory *inv, Recipes::Recipe recipe) {

        // Check feasibility
        for (const auto &input : recipe.inputs) {
            if (inv->getItem(input.id) < input.amount) {
                notify(std::format("Not enough items: {}", input.id));
                return false;
            }
        }

        // Execute craft
        for (const auto &input : recipe.inputs) {
            inv->subtractItem(input.id, input.amount);
        }
        for (const auto &output : recipe.outputs) {
            inv->addItem(output.id, output.amount);
        }
        return true;
    }

  public:
    virtual ~MainScreen() override = default;

    MainScreen() :
        Screen(),
        // Initialize reference_wrapper members here
        notifyText(putText(LINES - 1, 0, "")),
        inventoryWindow(createWindow(0, 0, COLS, 5, true, GAME_COLORS::GRAY_BLACK)),
        inventoryContents({
            inventoryWindow.putText(1, 2, ""s, GAME_COLORS::WHITE_BLACK),
            inventoryWindow.putText(2, 2, ""s, GAME_COLORS::WHITE_BLACK),
            inventoryWindow.putText(3, 2, ""s, GAME_COLORS::WHITE_BLACK)
        }),
        craftingWindow(createWindow(5, 12, COLS - 12, LINES - 6, true, GAME_COLORS::YELLOW_BLACK)),
        upgradesWindow(createWindow(5, 12, COLS - 12, LINES - 6, false, GAME_COLORS::RED_BLACK)),
        sidebarCraftingWindow(createWindow(5, 0, 12, 3, true, GAME_COLORS::YELLOW_GRAY)),
        sidebarUpgradesWindow(createWindow(8, 0, 12, 3, true, GAME_COLORS::RED_BLACK)),
        windows({
            {CRAFTING, WindowGroup(craftingWindow, sidebarCraftingWindow, GAME_COLORS::YELLOW_GRAY, GAME_COLORS::YELLOW_BLACK)},
            {UPGRADES, WindowGroup(upgradesWindow, sidebarUpgradesWindow, GAME_COLORS::RED_GRAY, GAME_COLORS::RED_BLACK)}
        }),
        inventory(ResourceManager::instance().getResource(Inventory::RESOURCE_ID)),
        recipes(ResourceManager::instance().getResource(Recipes::RESOURCE_ID))
    {
        // The rest of your constructor body
        (void)inventoryWindow.setTitle("Inventory", Window::Alignment::CENTER, GAME_COLORS::YELLOW_BLACK);
        (void)upgradesWindow.setTitle("Upgrades", Window::Alignment::LEFT, GAME_COLORS::RED_BLACK, 1);
        upgradeOptions.emplace("example_upgrade", upgradesWindow.putText(1, 1, "Example"s));
        (void)craftingWindow.setTitle("Crafting", Window::Alignment::LEFT, GAME_COLORS::YELLOW_BLACK, 1);
        auto syncedRecipes = recipes->synchronize();
        Recipes *rec = static_cast<Recipes*>(syncedRecipes->get());
        using Items = Inventory::Items;
        addCraftingOption<std::string>('1', {
                        {GAME_COLORS::WHITE_BLACK, "[1] "s},
                        {GAME_COLORS::YELLOW_BLACK, "Iron Ingot 1x"s}
                    }, getOrThrow(rec->get(Items::IRON), "Invalid recipe"));
        addCraftingOption<std::string>('2', {
                    {GAME_COLORS::WHITE_BLACK, "[2] "s},
                    {GAME_COLORS::YELLOW_BLACK, "Copper Ingot 1x"s}
                }, getOrThrow(rec->get(Items::COPPER), "Invalid recipe"));
        addCraftingOption<std::string>('3', {
                    {GAME_COLORS::WHITE_BLACK, "[3] "s},
                    {GAME_COLORS::YELLOW_BLACK, "Iron Gear 1x "s},
                    {GAME_COLORS::GRAY_BLACK, "(requires: 4 Iron Ingot)"s}
                }, getOrThrow(rec->get(Items::IRON_GEAR), "Invalid recipe"));
        addCraftingOption<std::string>('4', {
                        {GAME_COLORS::WHITE_BLACK, "[4] "s},
                        {GAME_COLORS::YELLOW_BLACK, "Copper Wire 3x "s},
                        {GAME_COLORS::GRAY_BLACK, "(requires: 1 Copper Ingot)"s}
                    }, getOrThrow(rec->get(Items::COPPER_WIRE), "Invalid recipe"));
        addCraftingOption<std::string>('5', {
                        {GAME_COLORS::WHITE_BLACK, "[5] "s},
                        {GAME_COLORS::YELLOW_BLACK, "Motor 1x "s},
                        {GAME_COLORS::GRAY_BLACK, "(requires: 2 Iron Gear, 10 Copper Wire)"s}
                    }, getOrThrow(rec->get(Items::MOTOR), "Invalid recipe"));
        (void)sidebarCraftingWindow.putText(1, 1, "[C]rafting"s, GAME_COLORS::DEFAULT);
        (void)sidebarUpgradesWindow.putText(1, 1, "[U]pgrades"s, GAME_COLORS::DEFAULT);
    }

    static std::unique_ptr<Screen> create() { return std::make_unique<MainScreen>(); }

    void onTick() override {

        // Update screen elements
        if (notifyTime > 0) {
            notifyTime--;
        }
        if (notifyTime <= 0 && !notifyClear) {
            notifyText.reset();
            notifyClear = true;
        }
        refreshInventoryCounts();

        // Handle input
        char input = ScreenManager::instance().getInput();
        auto syncedInv = inventory->synchronize();
        Inventory *inv = static_cast<Inventory *>(syncedInv->get());
        // Process global screen inputs
        switch (input) {
        case 'q':
            ScreenManager::instance().requestExit();
            return;
        case 'C':
            switchWindow(CRAFTING);
            return;
        case 'U':
            switchWindow(UPGRADES);
            return;
        case '\t':
            rotateWindows();
            return;
        case -1:
            return;
        }
        // Process registered input listeners
        if (auto i = inputListeners.find(input); i != inputListeners.end()) {
            i->second(this, inv);
            return;
        }

        // Unknown command
        notify(std::format("Unknown command: {} ({:d})", input, input));
        return;

    }
};
