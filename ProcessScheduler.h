#pragma once

#include <memory>
#include <string>
#include <fstream>
#include "Process.h"

// Schedulers
#include "FCFSScheduler.h"
#include "RRScheduler.h" //THIS IS NEW

class ProcessScheduler {
public:
	static ProcessScheduler& getInstance();

	void init();
	void showProcessList() const;
	void showScreenList() const;
	void makeReportUtil() const;
	void start();
	void stop();

	std::shared_ptr<Process> fetchProcessByName(const std::string& name);

	void setNumCPU(int n) { numCPU = n; }
	void setSchedulerType(const std::string& s) { type = s; }
	void setQuantumCycles(int q) { quantumCycles = q; }
	void setBatchProcessFreq(int f) { batchProcessFreq = f; }
	void setMinIns(int m) { minIns = m; }
	void setMaxIns(int m) { maxIns = m; }
	void setDelayPerExec(int d) { delayPerExec = d; }
	void loadConfigFromFile(const std::string& filename);

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