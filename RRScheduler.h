#pragma once

#include "AScheduler.h"

class RRScheduler : public Scheduler {
public:
    RRScheduler(int cores, int quantum, IMemoryAllocator& allocator)
        : Scheduler(cores, allocator), quantumCycles(quantum) {
    }

    void addProcess(std::shared_ptr<Process> process) override;
    void schedulerThread() override;
    void cpuCoreThread(int coreID) override;

    void setQuantumCycles(int q) { quantumCycles = q; }

private:
    int quantumCycles;
    //size_t memPerProc;
};