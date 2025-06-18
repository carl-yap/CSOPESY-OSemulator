#pragma once

#include <memory>
#include <string>
#include "Process.h"

// Schedulers
#include "FCFSScheduler.h"

// TODO: Make .cpp file for this (currently a lazy implementation)
class ProcessScheduler {
public:
	static ProcessScheduler& getInstance() {
		static ProcessScheduler instance;
		return instance;
	}

	void init() {
		if (type == "FCFS") {
			scheduler = std::make_shared<FCFSScheduler>(4);
		} else {
			throw std::runtime_error("Unknown scheduler type: " + type);
		}
		scheduler->schedulerStart();
	}

	void showScreenList() const {
		scheduler->displayScreenList();
	}

	std::shared_ptr<Process> fetchProcessByName(const std::string& name) {
		if (scheduler) {
			for (const auto& process : scheduler->processList) {
				if (process->getName() == name) {
					return std::make_shared<Process>(*process);
				}
			}
		}
		return nullptr;
	}

private:
	ProcessScheduler() {};

	std::string type = "FCFS";
	std::shared_ptr<Scheduler> scheduler = nullptr;
};