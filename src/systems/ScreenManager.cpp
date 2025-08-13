#include "ScreenManager.hpp"
#include "../screens/MainScreen.hpp"

void ScreenManager::init() {
    std::println(stderr, "Registering screens...");
    // Create and setup ScreenManager and Screen
    ScreenManager &instance = ScreenManager::instance();
    std::unique_ptr<Screen> mainScreen = MainScreen::create();
    std::reference_wrapper<Screen> movedMainScreen = instance.registerScreen(std::move(mainScreen));
    instance.changeScreen(&movedMainScreen.get());
}

ScreenManager &ScreenManager::instance() {
    static ScreenManager instance;
    return instance;
}

Screen *ScreenManager::getCurrentScreen() const {
    return currentScreen;
}

std::reference_wrapper<Screen> ScreenManager::registerScreen(std::unique_ptr<Screen> &&screen) {
    screens.push_back(std::move(screen));
    return std::ref(*screens.back().get());
}

void ScreenManager::changeScreen(Screen *screen) {
    nextScreen = screen;
    screenChange = true;
}

char ScreenManager::getInput() {
    return getch();
}

void ScreenManager::onTick() {
    // Set the screen on the first run
    if (!currentScreen && nextScreen) {
        currentScreen = nextScreen;
        nextScreen = nullptr;
        screenChange = false;
    }

    // Throw if there is no screen set
    if (!currentScreen) throw std::runtime_error("ScreenManager is not initialized!");

    // Run a single frame
    if (screenChange && nextScreen) {
        currentScreen = nextScreen;
        nextScreen = nullptr;
        screenChange = false;
    }
    currentScreen->onTick();
    currentScreen->render();
}

ScreenManager::~ScreenManager() {
    endwin();
}
