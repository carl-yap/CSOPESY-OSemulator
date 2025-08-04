#pragma once

#include <ctime>
#include <memory>
#include "AConsole.h"
#include "Process.h"
#include "ProcessScheduler.h"

class ProcessConsole : public Console {
public:
	ProcessConsole(const String pName);
	ProcessConsole(const String pName, size_t memSize);

	void onEnabled() override;
	void display() const override;
	void process() override;

	void setCustomCommands(const std::vector<std::vector<std::string>> commands);

private:
	String command;
	std::shared_ptr<Process> p;
	bool custom = false; // Flag to indicate if custom commands are running
};