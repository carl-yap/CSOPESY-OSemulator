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
	typedef std::shared_ptr<SymbolTable> TablePtr;
	Process(int id, const std::string& n, int minIns, int maxIns, size_t memoryRequired, size_t numPages);
	void generateInstructionsBetween(int min, int max);
	void setCustomInstructions(CommandList cmds);

	State		getState() const;
	int			getPID() const;
	int         getCmdListSize() const;
	std::string getName() const;
	std::string getCounter() const;
	TimePoint	getStartTime() const;
	TimePoint	getEndTime() const;
	TimePoint	getArrivalTime() const;
	size_t		getMemoryRequired() const;
	TablePtr    getSymbolTable();
	
	//void        setAllocationIndex(void* i) { allocIndex = i; }
	//void*       getAllocationIndex() { return allocIndex; }
	void        setAllocation(bool a) { allocated = a; }
	bool		isAllocated() { return allocated; }

	//void        setNumPages(size_t n) { numPages = n; }
	size_t      getNumPages() const { return numPages; }

	void setState(State newState);
	void setStartTime(TimePoint startTime);
	void setEndTime(TimePoint endTime);

	void executeCurrentCommand(int core) const;
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

	// Symbol table
	TablePtr symbolTable;
	std::unordered_map<uintptr_t, std::string> variables;

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
	//void*	allocIndex;
	bool	allocated;
	size_t  numPages;
	std::unordered_map<void*, bool> pageTable;
};