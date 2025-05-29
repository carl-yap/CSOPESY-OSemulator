#pragma once

#include <ctime>
#include "AConsole.h"

class ProcessConsole : public Console {
public:
	ProcessConsole() : Console("PROCESS_CONSOLE") {}

	void onEnabled() override {}

	void display() const override {
		std::cout << "Process Name: " << processName << std::endl;
		std::cout << "Current line of instruction: " << 50 << std::endl; // PLACEHOLDER
		std::cout << "Total lines of instruction: " << 50 << std::endl; // PLACEHOLDER
		std::cout << "Timestamp: " << getNow() << std::endl;
	}

	void process() override {
		display(); // TEMPORARY SOLUTION

		String command;
		std::cin >> command;
		// ConsoleManager::getInstance()->processCommand(command, this->name);
	}

	void setProcessName(const String& name) {
		this->processName = name;
	}

private:
	String processName = "N/A";

    String getNow() const {  
        char output[50];  
        std::time_t timestamp = std::time(nullptr);  
        struct tm dateTime;  

        if (localtime_s(&dateTime, &timestamp) == 0) {  
            strftime(output, 50, "%m/%d/%Y, %I:%M:%S %p", &dateTime);  
            return std::string(output);  
        } else {  
            return "Error retrieving time";  
        }  
    }
};