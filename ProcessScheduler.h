#pragma once

#include <memory>
#include <string>
#include "Process.h"

// Schedulers
#include "FCFSScheduler.h"

class ProcessScheduler {
public:
	static ProcessScheduler& getInstance();

	void init();
	void showScreenList() const;
	void makeReportUtil() const;

	std::shared_ptr<Process> fetchProcessByName(const std::string& name);

private:
	ProcessScheduler() {};
	std::shared_ptr<Scheduler> scheduler = nullptr;

	// Scheduler parameters (default)
	int			numCPU			= 4;
	std::string type			= "rr";
	int			quantumCycles	= 5; 
	int			batchProcessFreq = 1;
	int			minIns			= 1000;
	int			maxIns			= 2000;
	int			delayPerExec	= 0;
};