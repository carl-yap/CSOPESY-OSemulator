#include "MemoryAllocator.h"

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize, size_t memPerProcSize)
    : maxSize(maximumSize), allocatedSize(0), memPerProc(memPerProcSize) {
    memory.resize(maxSize, '.');  // initialize all memory blocks as free
    initializeMemory();

    // snapshot storage
	std::filesystem::create_directory("mem_snapshots");
}

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
    std::ostringstream out;

    // Timestamp
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[32];
    std::tm timeinfo;
    localtime_s(&timeinfo, &now);  // SAFE VERSION
    strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M:%S%p", &timeinfo);
    out << "Timestamp: (" << buf << ")\n";

    // Count processes
    size_t usedProcesses = 0;
    size_t externalFrag = 0;
    std::vector<std::tuple<size_t, int>> usedBlocks; // index, pid

    for (const auto& entry : processMap) {
		if (entry.second == -1) continue; // Skip unallocated blocks
        usedBlocks.push_back({ entry.first, entry.second });
    }

    usedProcesses = usedBlocks.size();

    // Count external fragmentation
    size_t freeBlockSize = 0;
    for (size_t i = 0; i < maxSize; i += memPerProc) {
        if (!allocationMap[i]) {
            freeBlockSize += memPerProc;
        }
    }

    // External fragmentation: unused memory that cannot fit a full process
    externalFrag = (freeBlockSize % memPerProc == 0) ? 0 : freeBlockSize % memPerProc;

    out << "Number of processes in memory: " << usedProcesses << "\n";
    out << "Total external fragmentation in KB: " << externalFrag / 1024 << "\n\n";

    out << "----end---- = " << maxSize << "\n\n";

    // Sort from high to low
    std::sort(usedBlocks.begin(), usedBlocks.end(), std::greater<>());

    for (const auto& [index, pid] : usedBlocks) {
        out << index + memPerProc << "\n";
        out << "P" << pid << "\n";
        out << index << "\n\n";
    }

    out << "----start---- = 0\n";

    return out.str();
}

/*============== Private Methods ================*/

void FlatMemoryAllocator::initializeMemory() {
	std::fill(memory.begin(), memory.end(), '.'); // '.' represents free memory
    for (size_t i = 0; i < maxSize; ++i) {
        allocationMap[i] = false;
    }
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
    processMap[index] = currentPID;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
	allocationMap[index] = false; // Mark the block as free
	for (size_t i = index; i < maxSize && allocationMap[i]; ++i) {
		memory[i] = '.'; // Reset to free memory representation
	}
    processMap[index] = -1;
}