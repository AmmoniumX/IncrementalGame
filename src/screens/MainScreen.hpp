#pragma once

#include "../ResourceManager.hpp"
#include "../render/Screen.hpp"
#include "../render/Text.hpp"
#include "../render/Window.hpp"
#include "../resources/Inventory.hpp"
#include "../ScreenManager.hpp"
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

    std::weak_ptr<Resource> inventory;

    Subwindows activeWindow = CRAFTING;

    template<size_t NumIn, size_t NumOut>
    struct Recipe {
        const std::array<const Inventory::ItemStack, NumIn> inputs;
        const std::array<const Inventory::ItemStack, NumOut> outputs;
    };

    struct Recipes {
        static inline const Recipe<0, 1> IRON_INGOT = {
            {},
            {Inventory::ItemStack(Inventory::Items::IRON, 1)}
        };
        static inline const Recipe<0, 1> COPPER_INGOT = {
            {},
            {Inventory::ItemStack(Inventory::Items::COPPER, 1)}
        };
        static inline const Recipe<1, 1> IRON_GEAR = {
            {Inventory::ItemStack(Inventory::Items::IRON, 4)},
            {Inventory::ItemStack(Inventory::Items::IRON_GEAR, 1)}
        };
        static inline const Recipe<1, 1> COPPER_WIRE = {
            {Inventory::ItemStack(Inventory::Items::COPPER, 1)},
            {Inventory::ItemStack(Inventory::Items::COPPER_WIRE, 3)}
        };
        static inline const Recipe<2, 1> MOTOR = {
            {Inventory::ItemStack(Inventory::Items::IRON_GEAR, 2),
                 Inventory::ItemStack(Inventory::Items::COPPER_WIRE, 10)},
            {Inventory::ItemStack(Inventory::Items::MOTOR, 1)}
        };
    };

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

    void refreshInventoryCounts() {
        auto sharedInv = inventory.lock();
        if (!sharedInv) {
            std::println(stderr, "ERROR: MainScreen: unable to get inventory");
            return;
        }
        auto syncedInv = (*sharedInv).synchronize();
        Inventory *inv = static_cast<Inventory *>(*syncedInv);

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
    template<TextString T, size_t NumIn, size_t NumOut>
    void addCraftingOption(char input, std::initializer_list<Text::TextChunk<T>> init, 
            Recipe<NumIn, NumOut> recipe) {
        craftingWindow.putText<std::string>(++numCraftingOptions, 1, init);
        registerListener(input, [recipe](MainScreen *scr, Inventory *inv) {
            scr->attemptRecipe(inv, recipe);
        });
    }

    template<size_t NumIn, size_t NumOut>
        requires(NumOut > 0)
    bool attemptRecipe(Inventory *inv, Recipe<NumIn, NumOut> recipe) {

        // Check feasibility
        if constexpr (NumIn > 0) {
            for (const auto &input : recipe.inputs) {
                if (inv->getItem(input.id) < input.amount) {
                    notify(std::format("Not enough items: {}", input.id));
                    return false;
                }
            }
        }

        // Execute craft
        if constexpr (NumIn > 0) {
            for (const auto &input : recipe.inputs) {
                inv->subtractItem(input.id, input.amount);
            }
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
        inventory(ResourceManager::instance().getResource(Inventory::RESOURCE_ID))
    {
        // The rest of your constructor body
        (void)inventoryWindow.setTitle("Inventory", Window::Alignment::CENTER, GAME_COLORS::YELLOW_BLACK);
        (void)upgradesWindow.setTitle("Upgrades", Window::Alignment::LEFT, GAME_COLORS::RED_BLACK, 1);
        upgradeOptions.emplace("example_upgrade", upgradesWindow.putText(1, 1, "Example"s));
        (void)craftingWindow.setTitle("Crafting", Window::Alignment::LEFT, GAME_COLORS::YELLOW_BLACK, 1);
        addCraftingOption<std::string, 0, 1>('1', {
                        {GAME_COLORS::WHITE_BLACK, "[1] "s},
                        {GAME_COLORS::YELLOW_BLACK, "Iron Ingot 1x"s}
                    }, Recipes::IRON_INGOT);
        addCraftingOption<std::string, 0, 1>('2', {
                    {GAME_COLORS::WHITE_BLACK, "[2] "s},
                    {GAME_COLORS::YELLOW_BLACK, "Copper Ingot 1x"s}
                }, Recipes::COPPER_INGOT);
        addCraftingOption<std::string, 1, 1>('3', {
                    {GAME_COLORS::WHITE_BLACK, "[3] "s},
                    {GAME_COLORS::YELLOW_BLACK, "Iron Gear 1x "s},
                    {GAME_COLORS::GRAY_BLACK, "(requires: 4 Iron Ingot)"s}
                }, Recipes::IRON_GEAR);
        addCraftingOption<std::string, 1, 1>('4', {
                        {GAME_COLORS::WHITE_BLACK, "[4] "s},
                        {GAME_COLORS::YELLOW_BLACK, "Copper Wire 3x "s},
                        {GAME_COLORS::GRAY_BLACK, "(requires: 1 Copper Ingot)"s}
                    }, Recipes::COPPER_WIRE);
        addCraftingOption<std::string, 2, 1>('5', {
                        {GAME_COLORS::WHITE_BLACK, "[5] "s},
                        {GAME_COLORS::YELLOW_BLACK, "Motor 1x "s},
                        {GAME_COLORS::GRAY_BLACK, "(requires: 2 Iron Gear, 10 Copper Wire)"s}
                    }, Recipes::MOTOR);
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
        auto sharedInv = inventory.lock();
        if (!sharedInv) {
            std::println(stderr, "ERROR: MainScreen: unable to get inventory");
            return;
        }
        auto syncedInv = (*sharedInv).synchronize();
        Inventory *inv = static_cast<Inventory *>(*syncedInv);
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
