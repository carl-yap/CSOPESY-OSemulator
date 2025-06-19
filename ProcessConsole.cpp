#include "ProcessConsole.h"
#include "ConsoleManager.h"

ProcessConsole::ProcessConsole(const String pName) : Console("PROCESS_CONSOLE") {
	this->p = ProcessScheduler::getInstance().fetchProcessByName(pName);
}

void ProcessConsole::onEnabled() { /* no-op */ }

void ProcessConsole::display() const {
	std::cout << "Process: " << p->getName() << std::endl;
	std::cout << "ID: " << p->getPID() << std::endl;
	std::cout << "Current instruction line: " << p->getCounter() << std::endl;
	std::cout << "Lines of code: " << p->getCmdListSize() << std::endl;
	std::cout << std::endl;
	std::cout << "Created at " << p->getArrivalTime() << std::endl;
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
