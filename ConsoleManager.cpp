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
    mainConsole = std::make_shared<MainConsole>();
    currentConsole = mainConsole;
}

// Show the main console screen
void ConsoleManager::showMainConsole() {
    currentConsole = mainConsole;
    currentConsole->display();
}

// Open a new or existing process screen
void ConsoleManager::openScreen(const std::string& name, bool resume) {
  /* 
    i think in this part we have to check if the screen name exist, so resume siya if true
	otherwise, pag di siya nag eexist, we can create a new ProcessConsole instance then set siguro yung process name after
    then i think need rin siya i store sa consoles. i honestly dont know what more we need, but yeahh this is basically
    the main part of the assignment. sorry if i made this without ur knowledge huhuhuhu </3
        -gab with annabelle pfp in discord <3
  if ()) {
        
    }
    else {
        
    }
    */
}

// Check if currently on the main console
bool ConsoleManager::isMainConsole() const {
    return currentConsole == mainConsole;
}
