#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <chrono>
#include <vector>
#include <string>

// #include "Process.h"
#include "Commands.h"
// temporary Process definition
struct Process {
	int pid;
	std::string name;
	std::vector<ICommand*> instructions;
	int currInst{ 0 };
	int instructionsRemaining;
	std::string logFilename;
	std::chrono::system_clock::time_point startTime;
	Process(int id, const std::string& n)
		: pid(id), name(n), instructionsRemaining(100) {
		for (int i = 0; i < instructionsRemaining; ++i) {
			std::string message = "Hello world from " + name + "!";
			instructions.emplace_back(new PrintCommand(pid, message));
		}

		logFilename = "processLog_" + name + ".txt";
		startTime = std::chrono::system_clock::now();
	}

	std::string executeInstruction() {
		if (currInst >= instructions.size()) {
			throw std::out_of_range("Instruction index out of bounds");
		}
		instructions[currInst]->execute();
		currInst += 1;
		return currInst < instructions.size() ? instructions[currInst]->getOutput() : "";
	}
};

// abstract class for schedulers: FCFS, RR, SJN
class Scheduler {
protected:
	inline static std::queue<Process> readyQueue;
	inline static std::mutex queueMutex;
	inline static std::condition_variable cvScheduler;

	int numCores;
	inline static std::vector<std::condition_variable> cvCores;
	inline static std::vector<std::mutex> coreMutexes;
	inline static std::vector<Process*> currentProcess; // current process running on each core
	inline static std::vector<std::unique_ptr<std::atomic_bool>> coreBusy;; // flags to check if a core is busy

	inline static std::atomic_bool running{ true };
	inline static std::atomic_int completedProcesses{ 0 }; // count of completed processes

	inline static std::mutex finishedMutex;
	inline static std::vector<Process> finishedProcesses;

	const int TOTAL_PROCESSES = 10; // temporary constant 

	std::string getTimestamp();
	void writeToLog(const std::string& filename, const std::string& message);
	void initializeLog(const Process& process);

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

		// create log directory
		std::filesystem::create_directory("logs");
		std::filesystem::current_path("logs");
	}
	~Scheduler() {
		// cleanup
		std::filesystem::current_path("..");
		for (int i = 0; i < numCores; ++i) {
			if (currentProcess[i]) {
				delete currentProcess[i]; // delete dynamically allocated processes
			}
		}
	}

	virtual void addProcess(const Process& process) = 0;
	virtual void schedulerThread() = 0; // 1 thread
	virtual void cpuCoreThread(int coreID) = 0; // {numCores} threads

	void displayScreenList() const;
	void schedulerStart();
	void schedulerStop();
};

