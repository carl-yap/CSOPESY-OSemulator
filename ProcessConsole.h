#pragma once

#include <ctime>
#include "AConsole.h"

class ProcessConsole : public Console {
public:
	ProcessConsole();

	void onEnabled() override;
	void display() const override;
	void process() override;

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
	String command;
	String processName = "DefaultProcess";
	String creationTime;
	int currentLine;
	int totalLines;

	String getNow() const;
};