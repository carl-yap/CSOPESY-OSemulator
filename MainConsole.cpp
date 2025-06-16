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
        else if (lowerCommand == "scheduler-test") {
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
        ConsoleManager::getInstance().showScreenList();
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

std::vector<Process> generateTestProcesses(int count) {
    std::vector<Process> processes;
    for (int i = 0; i < count; ++i) {
        processes.emplace_back(10 + i, "P" + std::to_string(i + 1));
    }
    return processes;
}


void MainConsole::handleSchedulerTest() {
    if (!scheduler) {
        scheduler = std::make_shared<FCFSScheduler>(4); // or your actual core count
        scheduler->start();

        auto testProcesses = generateTestProcesses(5);
        for (const auto& p : testProcesses) {
            scheduler->addProcess(p);
        }

        std::cout << "FCFS Scheduler started and processes submitted." << std::endl;
    }
    else {
        std::cout << "Scheduler already running." << std::endl;
    }
}


void MainConsole::handleSchedulerStop() {
    if (scheduler) {
        scheduler->stop();
        scheduler.reset();
        std::cout << "Scheduler stopped and cleaned up." << std::endl;
    }
    else {
        std::cout << "No scheduler running." << std::endl;
    }
}

void MainConsole::handleReportUtil() {
    std::cout << "report-util command recognized. Doing something." << std::endl;
}

void MainConsole::handleMarquee() {
    ConsoleManager::getInstance().openMarquee();
}