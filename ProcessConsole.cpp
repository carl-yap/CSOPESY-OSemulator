#include "ProcessConsole.h"
#include "ConsoleManager.h"

ProcessConsole::ProcessConsole(const String pName) : Console("PROCESS_CONSOLE") {
	this->p = ProcessScheduler::getInstance().fetchProcessByName(pName);
}

void ProcessConsole::onEnabled() { /* no-op */ }

void ProcessConsole::display() const {
    std::cout << "Process name: " << p->getName() << std::endl;
    std::cout << "ID: " << p->getPID() << std::endl;
    std::cout << "Logs:" << std::endl;

    // Print logs
    for (const auto& log : p->getLogs()) {
        // Format timestamp
        std::time_t t = std::chrono::system_clock::to_time_t(log.timestamp);
        std::tm tm;
        localtime_s(&tm, &t);
        char buf[32];
        std::strftime(buf, sizeof(buf), "(%m/%d/%Y %I:%M:%S%p)", &tm);

        std::cout << buf << " Core:" << log.core << " : " << log.message << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Current instruction line: " << p->getCounter() << std::endl;
    std::cout << "Lines of code: " << p->getCmdListSize() << std::endl;
    if (p->isFinished()) {
        std::cout << "Finished!" << std::endl;
    }
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
		else if (command == "process-smi") {
			// Print logs
			std::cout << "Process name: " << p->getName() << std::endl;
			std::cout << "ID: " << p->getPID() << std::endl;
			std::cout << "Logs:" << std::endl;
			for (const auto& log : p->getLogs()) {
				std::time_t t = std::chrono::system_clock::to_time_t(log.timestamp);
				std::tm tm;
				localtime_s(&tm, &t);
				char buf[32];
				std::strftime(buf, sizeof(buf), "(%m/%d/%Y %I:%M:%S%p)", &tm);

				std::cout << buf << " Core:" << log.core << " : " << log.message << std::endl;
			}
			std::cout << std::endl;
			std::cout << "Current instruction line: " << p->getCounter() << std::endl;
			std::cout << "Lines of code: " << p->getCmdListSize() << std::endl;
			if (p->isFinished()) {
				std::cout << "Finished!" << std::endl;
			}
		}
		else {
			std::cout << "Unknown command: " << command << std::endl;
		}
	}
	ConsoleManager::getInstance().clearScreen();
}
