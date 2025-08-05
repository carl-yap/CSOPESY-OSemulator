#include "ProcessConsole.h"
#include "ConsoleManager.h"

ProcessConsole::ProcessConsole(const String pName) : Console("PROCESS_CONSOLE") {
	this->p = ProcessScheduler::getInstance().fetchProcessByName(pName, 0);
}

ProcessConsole::ProcessConsole(const String pName, size_t memSize) : Console("PROCESS_CONSOLE") {
	this->p = ProcessScheduler::getInstance().fetchProcessByName(pName, memSize);
	ProcessScheduler::getInstance().addProcess(p);
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
		if (custom && p->getState() != Process::State::TERMINATED) {
			// If custom commands are running, we don't allow other commands
			std::cout << "Custom commands are running. Please wait until they finish." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(3));
			continue;
		}
		
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

void ProcessConsole::setCustomCommands(const std::vector<std::vector<std::string>> commands) {
	if (commands.empty()) {
		// std::cout << "No custom commands provided." << std::endl;
		return;
	}
	Process::CommandList commandList;

	// Loop through outer vector to read commands per line
	for (const auto& cmd : commands) {
		if (cmd.empty()) continue; // Skip empty command lines

		std::shared_ptr<ICommand> cPtr = nullptr;
		int pID = p->getPID();
		std::shared_ptr<SymbolTable> symbolTable = p->getSymbolTable();
		if (cmd[0] == "DECLARE") { // DECLARE varName value
			cPtr = std::make_shared<DeclareCommand>(pID, symbolTable, cmd[1], std::stoi(cmd[2]));
			commandList.push_back(cPtr);
			continue;
		} 
		else if (cmd[0] == "ADD") { // ADD out var1 var2
			cPtr = std::make_shared<AddCommand>(pID, symbolTable, cmd[1], cmd[2], cmd[3]);
			commandList.push_back(cPtr);
			continue;
		} 
		else if (cmd[0] == "READ") { // READ outVar addr
			uintptr_t address = static_cast<uintptr_t>(std::stoull(cmd[2], nullptr, 0));
			cPtr = std::make_shared<ReadCommand>(pID, symbolTable, cmd[1], address);
			commandList.push_back(cPtr);
			continue;
		}
		else if (cmd[0] == "WRITE") { // WRITE addr value 
			uintptr_t address = static_cast<uintptr_t>(std::stoull(cmd[1], nullptr, 0));
			if (std::all_of(cmd[2].begin(), cmd[2].end(), ::isdigit)) { // If value is a number
				uint16_t value = static_cast<uint16_t>(std::stoi(cmd[2]));
				cPtr = std::make_shared<WriteCommand>(pID, symbolTable, address, value);
			} else { // If value is a variable name
				cPtr = std::make_shared<WriteCommand>(pID, symbolTable, address, cmd[2]);
			}
			commandList.push_back(cPtr);
			continue;
		} 
		else if (cmd[0] == "PRINT") { // PRINT "message "
			cPtr = std::make_shared<PrintCommand>(pID, symbolTable, cmd);
			commandList.push_back(cPtr);
			continue;
		}
	}
	p->setCustomInstructions(commandList);
	ProcessScheduler::getInstance().addProcess(p);
	this->custom = true;
}