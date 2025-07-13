#pragma once

#include "AScheduler.h"

class FCFSScheduler : public Scheduler {
public:
	FCFSScheduler(int cores, IMemoryAllocator& allocator) : Scheduler(cores, allocator) {}

	void addProcess(std::shared_ptr<Process> process) override;
	void schedulerThread() override;
	void cpuCoreThread(int coreID) override;
};