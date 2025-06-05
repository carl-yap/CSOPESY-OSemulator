#include "ConsoleManager.h"

// Singleton instance
ConsoleManager& ConsoleManager::getInstance() {
    static ConsoleManager instance;
    return instance;
}

// Private constructor
ConsoleManager::ConsoleManager() {}

// Initialize the main console
void ConsoleManager::init() {
    clearScreen();
    mainConsole = std::make_shared<MainConsole>();
    currentConsole = mainConsole;
    currentConsole->display();
    this->running = true;
}

// Show the main console screen
void ConsoleManager::showMainConsole() {
    currentConsole = mainConsole;
}

void ConsoleManager::setRunStatus(bool running) {
    this->running = running;
}

// Returns false if the user types "exit" on main
bool ConsoleManager::isRunning() {
    return this->running;
}

// Open a new or existing process screen
void ConsoleManager::openScreen(const std::string& name, bool resume) {
    clearScreen();
    // Check if the screen name exists in our consoles map
    auto it = consoles.find(name);
    
    if (it != consoles.end() && resume) {
        // Screen exists and we want to resume it
        currentConsole = it->second;
        //currentConsole->display();
    }
    else if (it == consoles.end()) {
        // Screen doesn't exist
        auto newProcessConsole = std::make_shared<ProcessConsole>();
        newProcessConsole->setProcessName(name);
        
        
        consoles[name] = newProcessConsole;
        
      
        currentConsole = newProcessConsole;
        //currentConsole->display();
    }
    else {
        // Screen exists but resume is false 
        currentConsole = it->second;
        //currentConsole->display();
    }
}

// Check if currently on the main console
bool ConsoleManager::isMainConsole() const {
    return currentConsole == mainConsole;
}

void ConsoleManager::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}