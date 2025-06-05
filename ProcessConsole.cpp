#include "ProcessConsole.h"
#include "ConsoleManager.h"

ProcessConsole::ProcessConsole() : Console("PROCESS_CONSOLE") {
	this->creationTime = getNow();
	this->currentLine = 1;
	this->totalLines = 50;
}

void ProcessConsole::onEnabled() { /* no-op */ }

void ProcessConsole::display() const {
	std::cout << "Process: " << processName << std::endl;
	std::cout << "ID: " << processName << std::endl;
	std::cout << "Current instruction line: " << currentLine << std::endl;
	std::cout << "Lines of code: " << totalLines << std::endl;
	std::cout << std::endl;
	std::cout << "Timestamp: " << creationTime << std::endl;
	std::cout << std::endl;
	std::cout << "Type 'exit' to return to main menu." << std::endl;
}

void ProcessConsole::process() {
	while (ConsoleManager::getInstance().getCurrentConsole()->getName() == "PROCESS_CONSOLE") {
		std::cout << "Enter a command: ";
		std::getline(std::cin, command);

		if (command == "exit") {
			ConsoleManager::getInstance().showMainConsole();
		}
		else {
			std::cout << "Unknown command: " << command << std::endl;
		}
	}
	ConsoleManager::getInstance().clearScreen();
}

Console::String ProcessConsole::getNow() const {
	char output[50];
	std::time_t timestamp = std::time(nullptr);
	struct tm dateTime;

	if (localtime_s(&dateTime, &timestamp) == 0) {
		strftime(output, 50, "%m/%d/%Y, %I:%M:%S %p", &dateTime);
		return std::string(output);
	}
	else {
		return "Error retrieving time";
	}
}