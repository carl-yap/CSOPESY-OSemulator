#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <filesystem>

#include "Process.h"

// interface class
class IMemoryAllocator {
public:
	virtual void* allocate(std::shared_ptr<Process> process) = 0;
	virtual void deallocate(std::shared_ptr<Process> process) = 0;
	virtual std::string visualizeMemory() = 0;
};

class FlatMemoryAllocator : public IMemoryAllocator {
public:
	FlatMemoryAllocator(size_t maximumSize, size_t memPerProcSize);

	~FlatMemoryAllocator() {
		memory.clear();
	}

	void* allocate(size_t size);
	void deallocate(void* ptr);
	std::string visualizeMemory() override;
	void setCurrentPID(int pid) { currentPID = pid; }

private:
	size_t maxSize = 0;
	size_t allocatedSize = 0;
	size_t memPerProc = 4096;
	int currentPID = -1;

	std::vector<char> memory;
	std::unordered_map<size_t, bool> allocationMap; 
	std::unordered_map<size_t, int> processMap;

	void initializeMemory();
	bool canAllocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size);
	void deallocateAt(size_t index);

};

class PagingAllocator : public IMemoryAllocator {
public:
	PagingAllocator(size_t maxMemorySize, size_t numFrames);

	void* allocate(std::shared_ptr<Process> process) override;
	void deallocate(std::shared_ptr<Process> process) override;
	std::string visualizeMemory() override;

	// Add these new methods for vmstat
	size_t getUsedFrames() const { return frameMap.size(); }
	size_t getTotalFrames() const { return numFrames; }
	size_t getFreeFrames() const { return freeFrameList.size(); }
	size_t getNumPagedIn() const { return numPagedIn; }
	size_t getNumPagedOut() const { return numPagedOut; }

private:
	size_t maxMemorySize;
	size_t numFrames;
	std::unordered_map<size_t, size_t> frameMap;
	std::vector<size_t> freeFrameList;

	// Add paging counters
	mutable std::atomic<size_t> numPagedIn{ 0 };
	mutable std::atomic<size_t> numPagedOut{ 0 };

	size_t allocateFrames(size_t numFrames, size_t processID);
	void deallocateFrames(size_t numFrames, size_t frameIndex);
};