#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "AConsole.h"
#include "MainConsole.h"
#include "ProcessConsole.h"

class ConsoleManager {
public:
    static ConsoleManager& getInstance();

    void init();  // Initialize main console
    void showMainConsole();  // Return to main menu
    void openScreen(const std::string& name, bool resume);  // Open/resume process screen

    bool isMainConsole() const;

private:
    ConsoleManager();  // Private constructor (singleton)
    
    std::shared_ptr<Console> currentConsole;
    std::shared_ptr<MainConsole> mainConsole;
    std::unordered_map<std::string, std::shared_ptr<ProcessConsole>> consoles; // Name → ProcessConsole

};
