#include "MainConsole.h"
#include "ConsoleManager.h"
#include "ProcessScheduler.h"

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

        std::vector<std::string> commandTokens = tokenizeString(command);
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
        else if (lowerCommand == "scheduler-start") {
            handleSchedulerTest();
        }
        else if (lowerCommand == "scheduler-stop") {
            handleSchedulerStop();
        }
        else if (lowerCommand == "report-util") {
            handleReportUtil();
        }
        else if (lowerCommand == "marquee") {
            handleMarquee();
        }
        else {
            std::cout << "Unknown command. Available commands: initialize, marquee, screen, scheduler-test, scheduler-stop, report-util, clear, exit" << std::endl;
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

std::vector<std::string> MainConsole::tokenizeString(const String& input) {
    std::istringstream stream(input);
    return {std::istream_iterator<String>(stream), std::istream_iterator<String>()};
}

void MainConsole::handleInitialize() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
}

void MainConsole::handleScreen(std::vector<std::string> commandTokens) {
    bool resume = false;

    switch (commandTokens.size()) {
    case 1:
        std::cout << "screen command recognized. No additional parameters provided." << std::endl;
		break;
    case 2:
        if (commandTokens[1] != "-ls") {
            std::cout << "Process name is required. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
            break;
		}
        ProcessScheduler::getInstance().showScreenList();
        return;
    case 3:
        if (commandTokens[1] != "-r" && commandTokens[1] != "-s") {
            std::cout << "Syntax for screen command is incorrect. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
            break;
        }
        resume = (commandTokens[1] == "-r");
        ConsoleManager::getInstance().openScreen(commandTokens[2], resume);
        return;
    default:
        std::cout << "Too many parameters provided. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
		return;
    }
}

void MainConsole::handleSchedulerTest() {
	// This is temporary while scheduler-start is not implemented.
    ProcessScheduler& ps = ProcessScheduler::getInstance();
    ps.loadConfigFromFile("config.txt");  // Load before init
    ps.init(); // Use loaded settings
}

void MainConsole::handleSchedulerStop() {
    std::cout << "scheduler-stop command recognized. Shutting down scheduler." << std::endl;
    ProcessScheduler::getInstance().stop();
}

void MainConsole::handleReportUtil() {
    ProcessScheduler::getInstance().makeReportUtil();
}

void MainConsole::handleMarquee() {
    ConsoleManager::getInstance().openMarquee();
}