#pragma once

#include <ctime>
#include <memory>
#include "AConsole.h"
#include "Process.h"
#include "ProcessScheduler.h"

class ProcessConsole : public Console {
public:
	ProcessConsole(const String pName);

	void onEnabled() override;
	void display() const override;
	void process() override;

private:
	String command;
	std::shared_ptr<Process> p;
};