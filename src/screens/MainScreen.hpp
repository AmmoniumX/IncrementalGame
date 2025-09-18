#pragma once

#include "../render/Screen.hpp"
#include "../render/Text.hpp"
#include "../render/Window.hpp"
#include "../resources/Recipes.hpp"
#include "../resources/SaveData.hpp"
#include <array>
#include <cmath>
#include <functional>
#include <map>
#include <print>
#include <unordered_map>

using namespace std::string_literals;

class MainScreen : public Screen {
private:
  static inline const int NOTIF_DURATION = static_cast<int>(1.5 * FRAME_RATE);
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

  enum Subwindows { CRAFTING = 0, UPGRADES = 1 };
  struct WindowGroup {
    std::reference_wrapper<Window> main;
    std::reference_wrapper<Window> sidebar;
    int activeColor;
    int inactiveColor;
    WindowGroup() = delete; // delete default constructor: invalid with
                            // std::reference_wrapper
    WindowGroup(Window &m, Window &s, int a, int i)
        : main(m), sidebar(s), activeColor(a), inactiveColor(i) {}
  };

  std::unordered_map<Subwindows, WindowGroup> windows;

  SaveData &save;
  Recipes &recipes;

  Subwindows activeWindow = CRAFTING;

  void notify(const std::string &text);

  void rotateWindows();

  void switchWindow(Subwindows target);

  template <typename T>
  T getOrThrow(const std::optional<T> &opt, const std::string &msg) {
    if (!opt)
      throw std::runtime_error(msg);
    return *opt;
  }

  void refreshInventoryCounts();

  std::unordered_map<char, std::function<void(MainScreen *, SaveData &)>>
      inputListeners;

  void registerListener(char input,
                        std::function<void(MainScreen *, SaveData &)> listener);

  int numCraftingOptions = 0;
  template <TextString T>
  void addCraftingOption(char input,
                         std::initializer_list<Text::TextChunk<T>> init,
                         Recipes::Recipe recipe) {
    craftingWindow.putText<std::string>(++numCraftingOptions, 1, init);
    registerListener(input, [recipe](MainScreen *scr, SaveData &save) {
      scr->attemptRecipe(save, recipe);
    });
  }

  bool attemptRecipe(SaveData &save, Recipes::Recipe recipe);

public:
  virtual ~MainScreen() override = default;

  MainScreen();
  static std::unique_ptr<Screen> create();

  void onTick() override;
};
