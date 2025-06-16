#pragma once

#include <unordered_map>
#include <memory>
#include <string>

// Console layouts
#include "MainConsole.h"
#include "ProcessConsole.h"
#include "MarqueeConsole.h"

// Schedulers
#include "FCFSScheduler.h"

class ConsoleManager {
public:
    static ConsoleManager& getInstance();

    void init();  // Initialize main console
    void showMainConsole();  // Return to main menu
    void showScreenList() const;
    void openScreen(const std::string& name, bool resume);  // Open/resume process screen
    void openMarquee(); // startup the marquee console

    void setRunStatus(bool running);
    bool isRunning();

    bool isMainConsole() const;

    std::shared_ptr<Console> getCurrentConsole() const { return currentConsole; }

    void clearScreen();

private:
    ConsoleManager();  // Private constructor (singleton)
    
    std::shared_ptr<Console> currentConsole;
    std::shared_ptr<MainConsole> mainConsole;
    std::unordered_map<std::string, std::shared_ptr<ProcessConsole>> consoles; // Name → ProcessConsole

	Scheduler* scheduler = nullptr;

    bool running = false;
};