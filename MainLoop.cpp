#include <iostream>
#include <string>
#include <cctype>
#include <limits>


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

    displayHeader();
}

std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

void handleInitialize() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
}

void handleScreen() {
    std::cout << "screen command recognized. Doing something." << std::endl;
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

    std::string command;
    bool running = true;

    while (running) {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        std::string lowerCommand = toLower(command);

        if (lowerCommand == "exit") {
            std::cout << "exit command recognized. Exiting application." << std::endl;
            running = false;
        }
        else if (lowerCommand == "clear") {
            clearScreen();
        }
        else if (lowerCommand == "initialize") {
            handleInitialize();
        }
        else if (lowerCommand == "screen") {
            handleScreen();
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