#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

#include "Process.h"

// interface class
class IMemoryAllocator {
public:
	virtual void* allocate(size_t size) = 0;
	virtual void deallocate(void* ptr) = 0;
	virtual std::string visualizeMemory() = 0;
};

class FlatMemoryAllocator : public IMemoryAllocator {
public:
	FlatMemoryAllocator(size_t maximumSize);
	FlatMemoryAllocator(size_t maximumSize, size_t memPerProcSize);

	~FlatMemoryAllocator() {
		memory.clear();
	}

	void* allocate(size_t size) override;
	void deallocate(void* ptr) override;
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