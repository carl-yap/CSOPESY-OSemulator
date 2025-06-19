#include "ProcessScheduler.h"

ProcessScheduler& ProcessScheduler::getInstance() {
	static ProcessScheduler instance;
	return instance;
}

void ProcessScheduler::init() {
	if (type == "fcfs") {
		scheduler = std::make_shared<FCFSScheduler>(numCPU);
	}
	else if (type == "rr") {
		scheduler = std::make_shared<FCFSScheduler>(numCPU); // TODO: change
	}
	else {
		throw std::runtime_error("Unknown scheduler type: " + type);
	}
	//scheduler->schedulerStart();
}

void ProcessScheduler::showScreenList() const {
	std::cout << scheduler->displayScreenList().str();
}

void ProcessScheduler::makeReportUtil() const {
	std::string filename = "csopesy_log.txt";
	// Make the file then write the displayScreenList to it
}

std::shared_ptr<Process> ProcessScheduler::fetchProcessByName(const std::string& name) {
	if (scheduler) {
		for (const auto& process : scheduler->processList) {
			if (process->getName() == name) {
				return std::make_shared<Process>(*process);
			}
		}
	}
	return nullptr;
}