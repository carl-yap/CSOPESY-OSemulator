#pragma once

#include <ctime>
#include "AConsole.h"

class ProcessConsole : public Console {
public:
	ProcessConsole() : Console("PROCESS_CONSOLE") {
		creationTime = getNow();
		currentLine = 1;
		totalLines = 50;
	}

	void onEnabled() override {}

	void display() const override {
		std::cout << "Process: " << processName << std::endl;
		std::cout << "ID: " << processName << std::endl; 
		std::cout << "Current instruction line: " << currentLine << std::endl;
		std::cout << "Lines of code: " << totalLines << std::endl;
		std::cout << std::endl;
		std::cout << "Timestamp: " << creationTime << std::endl;
		std::cout << std::endl;
		std::cout << "Type 'exit' to return to main menu." << std::endl;
	}

	void process() override {
	}

	void setProcessName(const String& name) {
		this->processName = name;
	}

	String getProcessName() const {
		return processName;
	}

	void setCurrentLine(int line) {
		currentLine = line;
	}

	void setTotalLines(int lines) {
		totalLines = lines;
	}

private:
	String processName = "DefaultProcess";
	String creationTime;
	int currentLine;
	int totalLines;

	String getNow() const {
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
};