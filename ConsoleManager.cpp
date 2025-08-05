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
void ConsoleManager::openScreen(const std::string& name) {
    // Check if the screen name exists in our consoles map
    auto it = consoles.find(name);
    
    if (it != consoles.end()) {
        clearScreen();
        currentConsole = it->second; // Screen exists and we want to resume it
    } else {
        std::cerr << "Process '" << name << "' does not exist." << std::endl;
	}
}

void ConsoleManager::startScreen(const std::string& name, size_t memorySize) {
    // Check if the screen name exists in our consoles map
    auto it = consoles.find(name);
    
    if (it != consoles.end()) {
        std::cerr << "Process '" << name << "' already exists. Use resume to continue." << std::endl;
        return;
    }
    // Screen doesn't exist, start a new one
    auto newProcessConsole = std::make_shared<ProcessConsole>(name, memorySize);
    consoles[name] = newProcessConsole;
    currentConsole = newProcessConsole;
}

void ConsoleManager::customScreen(const std::string& name, const std::vector<std::vector<std::string>> commands) {
    // Check if the screen name exists in our consoles map
    auto it = consoles.find(name);
    
    if (it != consoles.end()) {
        std::cerr << "Process '" << name << "' already exists. Use resume to continue." << std::endl;
        return;
    }

    // Screen doesn't exist, start a new one with custom commands
    auto newProcessConsole = std::make_shared<ProcessConsole>(name);
    newProcessConsole->setCustomCommands(commands);

    consoles[name] = newProcessConsole;

	currentConsole = newProcessConsole;
}

void ConsoleManager::openMarquee() {
    clearScreen();
    currentConsole = std::make_unique<MarqueeConsole>();
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