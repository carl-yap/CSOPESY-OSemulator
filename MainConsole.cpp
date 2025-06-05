#include "MainConsole.h"
#include "ConsoleManager.h"

MainConsole::MainConsole() : Console("MAIN_CONSOLE") {
    this->command = "";
}

void MainConsole::onEnabled() { /* no-op */ }

void MainConsole::display() const {
    std::cout << "  ___  ____   __  ____  ____  ____  _  _ " << std::endl;
    std::cout << " / __)/ ___) /  \\(  _ \\(  __)/ ___)( \\/ )" << std::endl;
    std::cout << "( (__ \\___ \\(  O )) __/ ) _) \\___ \\ )  / " << std::endl;
    std::cout << " \\___)(____/ \\__/(__)  (____)(____/(__/  " << std::endl;
    std::cout << "Hello, Welcome to CSOPESPY commandline!" << std::endl;
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen" << std::endl;
}

void MainConsole::process() {
	while (ConsoleManager::getInstance().getCurrentConsole()->getName() == "MAIN_CONSOLE") {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        std::string commandTokens[10];
        tokenizeString(command, commandTokens);
        std::string lowerCommand = toLower(commandTokens[0]);

        if (lowerCommand == "exit") {
            std::cout << "exit command recognized. Exiting application." << std::endl;
            ConsoleManager::getInstance().setRunStatus(false);
            return;
        }
        else if (lowerCommand == "clear") {
		    ConsoleManager::getInstance().clearScreen();
            display();
        }
        else if (lowerCommand == "initialize") {
            handleInitialize();
        }
        else if (lowerCommand == "screen") {
            handleScreen(commandTokens);
        }
        else if (lowerCommand == "scheduler-test") {
            handleSchedulerTest();
        }
        else if (lowerCommand == "scheduler-stop") {
            handleSchedulerStop();
        }
        else if (lowerCommand == "report-util") {
            handleReportUtil();
        }
        else {
            std::cout << "Unknown command. Available commands: initialize, screen, scheduler-test, scheduler-stop, report-util, clear, exit" << std::endl;
        }
    }
    ConsoleManager::getInstance().clearScreen();
}

Console::String MainConsole::toLower(const String& str) {
    String result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

void MainConsole::tokenizeString(const String& input, String tokenArray[], int maxTokens) {
    std::istringstream stream(input);
    std::string token;
    int index = 0;

    // Initialize array
    for (int i = 0; i < maxTokens; i++) {
        tokenArray[i] = "";
    }

    while (std::getline(stream, token, ' ') && index < maxTokens) {
        tokenArray[index++] = token;
    }
}

void MainConsole::handleInitialize() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
}

void MainConsole::handleScreen(const String commandTokens[]) {
    if (commandTokens[1] == "" || (commandTokens[1] != "-r" && commandTokens[1] != "-s")) {
        std::cout << "Syntax for screen command is incorrect. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
        return;
    }

    if (commandTokens[2] == "") {
        std::cout << "Process name is required. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
        return;
    }

    bool resume = (commandTokens[1] == "-r");
    ConsoleManager::getInstance().openScreen(commandTokens[2], resume);
}

void MainConsole::handleSchedulerTest() {
    std::cout << "scheduler-test command recognized. Doing something." << std::endl;
}

void MainConsole::handleSchedulerStop() {
    std::cout << "scheduler-stop command recognized. Doing something." << std::endl;
}

void MainConsole::handleReportUtil() {
    std::cout << "report-util command recognized. Doing something." << std::endl;
}

void MainConsole::handleMarquee() {
	return; // Placeholder for future marquee functionality
}