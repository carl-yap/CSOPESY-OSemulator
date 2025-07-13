#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

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
	FlatMemoryAllocator(size_t maximumSize) : maxSize(maximumSize), allocatedSize(0) {
		memory.reserve(maximumSize);
		initializeMemory();
	}

	~FlatMemoryAllocator() {
		memory.clear();
	}

	void* allocate(size_t size) override;
	void deallocate(void* ptr) override;
	std::string visualizeMemory() override;

private:
	size_t maxSize = 0;
	size_t allocatedSize = 0;
	std::vector<char> memory;
	std::unordered_map<size_t, bool> allocationMap; 

	void initializeMemory();
	bool canAllocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size);
	void deallocateAt(size_t index);
};