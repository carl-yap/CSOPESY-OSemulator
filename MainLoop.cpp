#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <limits>

#include "ConsoleManager.h"

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

void tokenizeString(const std::string& input, std::string tokenArray[], int maxTokens = 10) {
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

void handleInitialize() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
}

void handleScreen(const std::string commandTokens[]) {
    if (commandTokens[1] == "" || (commandTokens[1] != "-r" && commandTokens[1] != "-s")) {
        std::cout << "Syntax for screen command is incorrect. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
        return;
    }

    if (commandTokens[2] == "") {
        std::cout << "Process name is required. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
        return;
    }

    clearScreen();

    bool resume = (commandTokens[1] == "-r");
    ConsoleManager::getInstance().openScreen(commandTokens[2], resume);
}

void handleSchedulerTest() {
    std::cout << "scheduler-test command recognized. Doing something." << std::endl;
}

void handleSchedulerStop() {
    std::cout << "scheduler-stop command recognized. Doing something." << std::endl;
}

void handleReportUtil() {
    std::cout << "report-util command recognized. Doing something." << std::endl;
}

int main() {
    clearScreen();

    ConsoleManager::getInstance().init();
    ConsoleManager::getInstance().showMainConsole();

    std::string command;
    bool running = true;

    while (running) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        std::string commandTokens[10];
        tokenizeString(command, commandTokens);
        std::string lowerCommand = toLower(commandTokens[0]);

        if (lowerCommand == "exit") {
            if (ConsoleManager::getInstance().isMainConsole()) {
                std::cout << "exit command recognized. Exiting application." << std::endl;
                running = false;
            }
            else {
               
                clearScreen();
                ConsoleManager::getInstance().showMainConsole();
            }
        }
        else if (lowerCommand == "clear") {
            clearScreen();
            if (ConsoleManager::getInstance().isMainConsole()) {
                ConsoleManager::getInstance().showMainConsole();
            }
            else {
                // Redisplay current console
                ConsoleManager::getInstance().getCurrentConsole()->display();
            }
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

    return 0;
}