#pragma once

#include <mutex>
#include <print>
#include <vector>

class System {
public:
    virtual ~System() = default;
    virtual void onInit();
    virtual void onTick();

    void requestExit();
};

class SystemManager {
private:
    SystemManager() = default;
    std::vector<System*> systems;
    std::mutex mtx;

public:

    static void init();

    static SystemManager &instance();

    void registerSystem(System *system);

    void onTick();
};

