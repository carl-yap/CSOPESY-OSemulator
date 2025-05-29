#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <limits>

// Temp w/o Console Manager
#include "AConsole.h"
#include "MainConsole.h"
#include "ProcessConsole.h"

void displayHeader() {
    std::cout << "  ___  ____   __  ____  ____  ____  _  _ " << std::endl;
    std::cout << " / __)/ ___) /  \\(  _ \\(  __)/ ___)( \\/ )" << std::endl;
    std::cout << "( (__ \\___ \\(  O )) __/ ) _) \\___ \\ )  / " << std::endl;
    std::cout << " \\___)(____/ \\__/(__)  (____)(____/(__/  " << std::endl;
    std::cout << "Hello, Welcome to CSOPESPY commandline!" << std::endl;
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen" << std::endl;
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    // displayHeader();
}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

void tokenizeString(std::string& input, std::string tokenArray[], int index = 0) {
    std::istringstream stream(input);
    std::string token;
    while (std::getline(stream, token, ' ')) {
        tokenArray[index++] = token;
    }
}

void handleInitialize() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
}

void handleScreen(std::string commandTokens[], ProcessConsole* proc) {
    if (commandTokens[1] == "" || !(commandTokens[1].compare("-r") == 0) && !(commandTokens[1].compare("-s") == 0))
    {
        std::cout << "Syntax for screen command is incorrect. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
        return;
    }

    clearScreen();
    proc->setProcessName(commandTokens[2]);
    proc->display();
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
    // Temporary console pointers
    Console* mainConsole = new MainConsole();
    Console* processConsole = new ProcessConsole();


    mainConsole->display();

    std::string command;
    bool running = true;

    while (running) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        std::string commandTokens[10];
        tokenizeString(command, commandTokens);
        std::string lowerCommand = commandTokens[0];

        if (lowerCommand == "exit") {
            std::cout << "exit command recognized. Exiting application." << std::endl;
            running = false;
        }
        else if (lowerCommand == "clear") {
            clearScreen();
            mainConsole->display();
        }
        else if (lowerCommand == "initialize") {
            handleInitialize();
        }
        else if (lowerCommand == "screen") {
            std::unique_ptr<ProcessConsole> ptr = std::make_unique<ProcessConsole>();
             handleScreen(commandTokens, ptr.get());
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