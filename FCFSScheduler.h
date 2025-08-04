#pragma once

#include "AScheduler.h"

class FCFSScheduler : public Scheduler {
public:
	FCFSScheduler(int cores, IMemoryAllocator& allocator, size_t memPerProc, int quantumCycles = 4)
		: Scheduler(cores, allocator), memPerProc(memPerProc), quantumCycles(quantumCycles) {
	}

	void addProcess(std::shared_ptr<Process> process) override;
	void schedulerThread() override;
	void cpuCoreThread(int coreID) override;

	void setQuantumCycles(int q) { quantumCycles = q; }

private:
	size_t memPerProc;
	int quantumCycles;
};