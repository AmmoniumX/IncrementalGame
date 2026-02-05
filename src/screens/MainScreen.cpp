#include <string>
#include <string_view>

#include "../Logger.hpp"
#include "../game.hpp"
#include "../systems/ScreenManager.hpp"

#include "MainScreen.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

void MainScreen::notify(std::string_view text) {
  Logger::println("{}", text);
  notifyText.setText(std::string{text}, true);
  notifyStart = Clock::now();
}

void MainScreen::rotateWindows() {
  WindowGroup w = windows.at(activeWindow);
  (w.main.get()).disable();
  (w.sidebar.get()).setColorPair(w.inactiveColor);
  activeWindow = Subwindows((activeWindow + 1) % windows.size());
  w = windows.at(activeWindow);
  (w.main.get()).enable();
  (w.sidebar.get()).setColorPair(w.activeColor);
}

void MainScreen::switchWindow(Subwindows target) {
  WindowGroup w = windows.at(activeWindow);
  if (target == activeWindow) {
    return;
  }
  (w.main.get()).disable();
  (w.sidebar.get()).setColorPair(w.inactiveColor);
  activeWindow = target;
  w = windows.at(activeWindow);
  (w.main.get()).enable();
  (w.sidebar.get()).setColorPair(w.activeColor);
}

void MainScreen::refreshInventoryCounts() {

  const SaveData::Map items = save.getItems();

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
        Logger::println("Inventory overflow, cannot display all items.");
        break;
      }
    } else {
      // Move to next line if current line is full
      currLine++;
      if (currLine < 3) {
        display_lines[currLine] = entry + " ";
      } else {
        Logger::println("Inventory overflow, cannot display all items.");
        break;
      }
    }
  }

  // Set display lines
  for (int i = 0; i < 3; i++) {
    auto oldText = inventoryContents[i].get().getText();
    if (oldText == display_lines[i]) {
      return;
    }
    inventoryContents[i].get().setText(display_lines[i], true,
                                       GAME_COLORS::WHITE_BLACK);
  }
}

void MainScreen::registerListener(
    char input, std::function<void(MainScreen *, SaveData &)> listener) {
  inputListeners.insert_or_assign(input, listener);
}

bool MainScreen::attemptRecipe(SaveData &save, Recipes::Recipe recipe) {

  // Check feasibility
  for (const auto &input : recipe.inputs) {
    if (save.getItem(input.id) < input.amount) {
      notify(std::format("Not enough items: {}", input.id));
      return false;
    }
  }

  // Execute craft
  for (const auto &input : recipe.inputs) {
    save.subtractItem(input.id, input.amount);
  }
  for (const auto &output : recipe.outputs) {
    save.addItem(output.id, output.amount);
  }
  return true;
}

MainScreen::MainScreen()
    : Screen(),
      // Initialize reference_wrapper members here
      notifyText(putText(LINES - 1, 0, "")), notifyStart{},
      inventoryWindow(
          createWindow(0, 0, COLS, 5, true, GAME_COLORS::GRAY_BLACK)),
      inventoryContents(
          {inventoryWindow.putText(1, 2, ""s, GAME_COLORS::WHITE_BLACK),
           inventoryWindow.putText(2, 2, ""s, GAME_COLORS::WHITE_BLACK),
           inventoryWindow.putText(3, 2, ""s, GAME_COLORS::WHITE_BLACK)}),
      craftingWindow(createWindow(5, 12, COLS - 12, LINES - 6, true,
                                  GAME_COLORS::YELLOW_BLACK)),
      upgradesWindow(createWindow(5, 12, COLS - 12, LINES - 6, false,
                                  GAME_COLORS::RED_BLACK)),
      sidebarCraftingWindow(
          createWindow(5, 0, 12, 3, true, GAME_COLORS::YELLOW_GRAY)),
      sidebarUpgradesWindow(
          createWindow(8, 0, 12, 3, true, GAME_COLORS::RED_BLACK)),
      windows({{CRAFTING, WindowGroup(craftingWindow, sidebarCraftingWindow,
                                      GAME_COLORS::YELLOW_GRAY,
                                      GAME_COLORS::YELLOW_BLACK)},
               {UPGRADES,
                WindowGroup(upgradesWindow, sidebarUpgradesWindow,
                            GAME_COLORS::RED_GRAY, GAME_COLORS::RED_BLACK)}}),
      save(SaveData::instance()), recipes(Recipes::instance()) {
  (void)inventoryWindow.setTitle("Inventory", Window::Alignment::CENTER,
                                 GAME_COLORS::YELLOW_BLACK);
  (void)upgradesWindow.setTitle("Upgrades", Window::Alignment::LEFT,
                                GAME_COLORS::RED_BLACK, 1);
  upgradeOptions.emplace("example_upgrade",
                         upgradesWindow.putText(1, 1, "Example"s));
  (void)craftingWindow.setTitle("Crafting", Window::Alignment::LEFT,
                                GAME_COLORS::YELLOW_BLACK, 1);
  using Items = SaveData::Items;
  addCraftingOption('1',
                    {{GAME_COLORS::WHITE_BLACK, "[1] "s},
                     {GAME_COLORS::YELLOW_BLACK, "Iron Ingot 1x"s}},
                    getOrThrow(recipes.get(Items::IRON), "Invalid recipe"sv));
  addCraftingOption('2',
                    {{GAME_COLORS::WHITE_BLACK, "[2] "s},
                     {GAME_COLORS::YELLOW_BLACK, "Copper Ingot 1x"s}},
                    getOrThrow(recipes.get(Items::COPPER), "Invalid recipe"sv));
  addCraftingOption(
      '3',
      {{GAME_COLORS::WHITE_BLACK, "[3] "s},
       {GAME_COLORS::YELLOW_BLACK, "Iron Gear 1x "s},
       {GAME_COLORS::GRAY_BLACK, "(requires: 4 Iron Ingot)"s}},
      getOrThrow(recipes.get(Items::IRON_GEAR), "Invalid recipe"sv));
  addCraftingOption(
      '4',
      {{GAME_COLORS::WHITE_BLACK, "[4] "s},
       {GAME_COLORS::YELLOW_BLACK, "Copper Wire 3x "s},
       {GAME_COLORS::GRAY_BLACK, "(requires: 1 Copper Ingot)"s}},
      getOrThrow(recipes.get(Items::COPPER_WIRE), "Invalid recipe"sv));
  addCraftingOption(
      '5',
      {{GAME_COLORS::WHITE_BLACK, "[5] "s},
       {GAME_COLORS::YELLOW_BLACK, "Motor 1x "s},
       {GAME_COLORS::GRAY_BLACK, "(requires: 2 Iron Gear, 10 Copper Wire)"s}},
      getOrThrow(recipes.get(Items::MOTOR), "Invalid recipe"sv));
  (void)sidebarCraftingWindow.putText(1, 1, "[C]rafting"s,
                                      GAME_COLORS::DEFAULT);
  (void)sidebarUpgradesWindow.putText(1, 1, "[U]pgrades"s,
                                      GAME_COLORS::DEFAULT);
}

std::unique_ptr<Screen> MainScreen::create() {
  return std::make_unique<MainScreen>();
}

void MainScreen::onTick() {

  // Update screen elements
  if (notifyStart.has_value()) {
    auto now = Clock::now();
    if (now - *notifyStart > NOTIF_DURATION) {
      Logger::println("Clearing notification");
      notifyText.reset();
      notifyStart.reset();
    }
  }
  refreshInventoryCounts();

  // Handle input
  char input = ScreenManager::instance().getInput();
  // Process global screen inputs
  switch (input) {
  case 'q':
    Logger::println("Requesting Exit");
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
    i->second(this, save);
    return;
  }

  // Unknown command
  notify(std::format("Unknown command: {} ({:d})", input, input));
  return;
}
