#pragma once

#include <memory>
#include <string>
#include <fstream>
#include "Process.h"

// Schedulers
#include "FCFSScheduler.h"
#include "RRScheduler.h"

class ProcessScheduler {
public:
    static ProcessScheduler& getInstance();

    void init();
    void showProcessList() const;
    void showScreenList() const;
    void makeReportUtil() const;
    void start();
    void stop();
    void exit(); // cleanup resources & threads

    std::shared_ptr<Process> fetchProcessByName(const std::string& name, size_t memSize);

    void setNumCPU(int n) { numCPU = n; }
    void setSchedulerType(const std::string& s) { type = s; }
    void setQuantumCycles(int q) { quantumCycles = q; }
    void setBatchProcessFreq(int f) { batchProcessFreq = f; }
    void setMinIns(int m) { minIns = m; }
    void setMaxIns(int m) { maxIns = m; }
    void setDelayPerExec(int d) { delayPerExec = d; }
    void setMaxOverallMem(size_t m) { maxOverallMem = m; }
    void setMemPerFrame(size_t m) { memPerFrame = m; }
    void setMinMemPerProc(size_t m) { minMemPerProc = m; }
    void setMaxMemPerProc(size_t m) { maxMemPerProc = m; }

    bool isValidMemorySize(size_t size) const;

    void loadConfigFromFile(const std::string& filename);

    void showVMStat() const;

	bool processExists(const std::string& name) const;

private:
    ProcessScheduler() {};
    std::shared_ptr<Scheduler> scheduler = nullptr;
    std::shared_ptr<DemandPagingAllocator> demandPagingAllocator;

    // Scheduler parameters (default)
    int         numCPU = 4;
    std::string type = "rr";
    int         quantumCycles = 5;
    int         batchProcessFreq = 1;
    int         minIns = 1000;
    int         maxIns = 2000;
    int         delayPerExec = 0;
    size_t      maxOverallMem = 16 * 1024;
    size_t      memPerFrame = 16;
    size_t      minMemPerProc = 4096;
    size_t      maxMemPerProc = 8192;
};
