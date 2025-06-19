#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <chrono>
#include <vector>
#include <string>

#include "Process.h"

// abstract class for schedulers: FCFS, RR, SJN
class Scheduler {
protected:
	inline static std::queue<std::shared_ptr<Process>> readyQueue;
	inline static std::mutex queueMutex;
	inline static std::condition_variable cvScheduler;

	int numCores;
	inline static std::vector<std::condition_variable> cvCores;
	inline static std::vector<std::mutex> coreMutexes;
	inline static std::vector<std::shared_ptr<Process>> currentProcess; // current process running on each core
	inline static std::vector<std::unique_ptr<std::atomic_bool>> coreBusy;; // flags to check if a core is busy

	inline static std::atomic_bool running{ true };
	inline static std::atomic_int completedProcesses{ 0 }; // count of completed processes

	inline static std::mutex finishedMutex;
	inline static std::vector<Process> finishedProcesses;

	const int TOTAL_PROCESSES = 10; // temporary constant 

	std::string getTimestamp();

public:
	Scheduler(int cores) : numCores(cores) {
		// initialize vectors based on the number of cores
		cvCores = std::vector<std::condition_variable>(numCores);
		coreMutexes = std::vector<std::mutex>(numCores);
		currentProcess.resize(numCores, nullptr);
		coreBusy.resize(numCores);
		for (int i = 0; i < numCores; ++i) {
			coreBusy[i] = std::make_unique<std::atomic_bool>(false);
		}
		
		for (int i = 0; i < TOTAL_PROCESSES; ++i) {
			processList.emplace_back(); // reserve space for finished processes
		}
	}
	~Scheduler() = default;

	virtual void addProcess(std::shared_ptr<Process> process) = 0;
	virtual void schedulerThread() = 0; // 1 thread
	virtual void cpuCoreThread(int coreID) = 0; // {numCores} threads

	// String stream for screen -ls
	std::ostringstream displayScreenList() const;

	void schedulerStart();
	void schedulerStop();

	std::vector<std::shared_ptr<Process>> processList;
};

