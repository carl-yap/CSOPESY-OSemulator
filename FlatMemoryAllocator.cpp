#include "MemoryAllocator.h"

/*============== Overriden Methods ================*/

void* FlatMemoryAllocator::allocate(size_t size) {
	// First-fit allocation strategy
	for (size_t i = 0; i < maxSize - size + 1; ++i) {
		if (!allocationMap[i] && canAllocateAt(i, size)) {
			allocateAt(i, size); // Fills the allocation map & updates allocated size
			return &memory[i];
		}
	}

	return nullptr; // No suitable block found
}

void FlatMemoryAllocator::deallocate(void* ptr) {
	// Index of memory block to deallocate
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index]) {
		deallocateAt(index); // marks the block in the allocation map as free
	}
}

std::string FlatMemoryAllocator::visualizeMemory() {
	// TODO: visualize the memory layout
}

/*============== Private Methods ================*/

void FlatMemoryAllocator::initializeMemory() {
	std::fill(memory.begin(), memory.end(), '.'); // '.' represents free memory
	std::fill(allocationMap.begin(), allocationMap.end(), false); 
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
	// if block is large enough
	return (index + size <= maxSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
	// Mark the memory block as allocated in the allocation map
	for (size_t i = index; i < index + size; ++i) {
		allocationMap[i] = true;
		memory[i] = 'X'; // 'X' represents allocated memory
	}
	allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
	allocationMap[index] = false; // Mark the block as free
	for (size_t i = index; i < maxSize && allocationMap[i]; ++i) {
		memory[i] = '.'; // Reset to free memory representation
	}
}