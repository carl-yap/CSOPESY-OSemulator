#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <memory>
#include "SymbolTable.h"
#include "Commands.h"

class Process {
public:
	enum class State {
		NEW,
		RUNNING,
		WAITING,
		READY,
		TERMINATED
	};

	typedef std::vector<std::shared_ptr<ICommand>> CommandList;
	typedef std::chrono::system_clock::time_point TimePoint;
	Process(int id, const std::string& n, int minIns, int maxIns, size_t memPerProc);
	void generateInstructionsBetween(int min, int max);

	State		getState() const;
	int			getPID() const;
	int         getCmdListSize() const;
	std::string getName() const;
	std::string getCounter() const;
	TimePoint	getStartTime() const;
	TimePoint	getEndTime() const;
	TimePoint	getArrivalTime() const;
	size_t		getMemoryRequired() const;
	
	void        setAllocationIndex(void* i) { allocIndex = i; }
	void*       getAllocationIndex() { return allocIndex; }
	void        setAllocation(bool a) { allocated = a; }
	bool		isAllocated() { return allocated; }

	void setState(State newState);
	void setStartTime(TimePoint startTime);
	void setEndTime(TimePoint endTime);

	void executeCurrentCommand() const;
	void moveToNextLine();

	// Get the remaining number of instructions 
	int getRemainingBurstTime() const;

	// Check if the process is finished
	bool isFinished() const;

	struct LogEntry {
		std::chrono::system_clock::time_point timestamp;
		int core;
		std::string message;
	};

	void addLog(int core, const std::string& message);
	const std::vector<LogEntry>& getLogs() const;

private:
	int pid;
	std::string name;
	State state;

	// Subroutines
	CommandList instructions;
	int programCounter;

	// Symbol table [Work in Progress]
	// SymbolTable symbolTable;

	// TODO: Stack
	// TODO: Heap

	// Scheduler tracking
	int turnaroundTime;
	int waitingTime;
	TimePoint arrivalTime;
	TimePoint startTime;
	TimePoint endTime;

	std::vector<LogEntry> logs;

	// Memory Allocation 
	size_t	memoryRequired;
	void*	allocIndex;
	bool	allocated;
};