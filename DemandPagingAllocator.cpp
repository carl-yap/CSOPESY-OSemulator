#include "MemoryAllocator.h"
#include <chrono>
#include <random>

DemandPagingAllocator::DemandPagingAllocator(size_t maxMemorySize, size_t frameSize, size_t maxFrames, const std::string& backingStoreFile)
    : maxMemorySize(maxMemorySize), frameSize(frameSize), maxFrames(maxFrames), backingStoreFile(backingStoreFile), nextBackingStoreOffset(0) {

    // Initialize physical memory frames
    frames.resize(maxFrames);
    for (size_t i = 0; i < maxFrames; ++i) {
        freeFrames.push(i);
    }

    // Initialize backing store
    initializeBackingStore();

}

DemandPagingAllocator::~DemandPagingAllocator() {
    if (backingStore.is_open()) {
        backingStore.close();
    }
}

void DemandPagingAllocator::initializeBackingStore() {
    // Create or open backing store file
    backingStore.open(backingStoreFile, std::ios::in | std::ios::out | std::ios::binary);
    if (!backingStore.is_open()) {
        // Create new file
        backingStore.open(backingStoreFile, std::ios::out | std::ios::binary);
        backingStore.close();
        backingStore.open(backingStoreFile, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!backingStore.is_open()) {
        std::cerr << "[DemandPaging] ERROR: Could not open backing store file: " << backingStoreFile << std::endl;
    }
}

void* DemandPagingAllocator::allocate(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(allocatorMutex);

    size_t processId = process->getPID();
    size_t numPages = process->getNumPages();

    // Create page table for the process (initially all pages not present)
    processPageTables[processId].resize(numPages);

    // In demand paging, we don't allocate physical frames immediately
    // Pages will be allocated when first accessed (page fault)
    // Return a non-null pointer to indicate successful virtual allocation
    return reinterpret_cast<void*>(processId + 1);
}

void DemandPagingAllocator::deallocate(std::shared_ptr<Process> process) {
    std::lock_guard<std::mutex> lock(allocatorMutex);

    size_t processId = process->getPID();

    auto pageTableIt = processPageTables.find(processId);
    if (pageTableIt == processPageTables.end()) {
        return; // Process not found
    }

    std::vector<PageTableEntry>& pageTable = pageTableIt->second;
    size_t deallocatedFrames = 0;

    // Free all physical frames and backing store slots used by this process
    for (size_t pageNum = 0; pageNum < pageTable.size(); ++pageNum) {
        PageTableEntry& entry = pageTable[pageNum];

        if (entry.present) {
            // Free physical frame
            size_t frameNum = entry.frameNumber;
            frames[frameNum].occupied = false;
            frames[frameNum].processId = 0;
            frames[frameNum].pageNumber = 0;
            freeFrames.push(frameNum);
            deallocatedFrames++;
            usedFrames--;
        }
        else if (entry.backingStoreOffset > 0) {
            // Free backing store slot
            deallocateBackingStoreSlot(entry.backingStoreOffset);
        }
    }

    // Remove process page table
    processPageTables.erase(pageTableIt);

    // Update statistics
    numPagedOut += deallocatedFrames;
}

bool DemandPagingAllocator::accessPage(size_t processId, size_t pageNumber, bool isWrite) {
    std::lock_guard<std::mutex> lock(allocatorMutex);

    auto pageTableIt = processPageTables.find(processId);
    if (pageTableIt == processPageTables.end() || pageNumber >= pageTableIt->second.size()) {
       return false;
    }

    PageTableEntry& entry = pageTableIt->second[pageNumber];

    // Check if page is present in physical memory
    if (entry.present) {
        // Page hit - update access information
        entry.referenced = true;
        if (isWrite) {
            entry.dirty = true;
        }
        frames[entry.frameNumber].lastAccessed = std::chrono::steady_clock::now();
        return true;
    }

    // Page fault occurred
    numPageFaults++;
    // Find a free frame or evict one
    size_t frameNumber;
    if (!freeFrames.empty()) {
        frameNumber = freeFrames.front();
        freeFrames.pop();
    }
    else {
        // No free frames, need to evict a page
        frameNumber = findVictimFrame();
        if (!swapOut(frameNumber)) {
            return false;
        }
    }

    // Load page into physical memory
    if (entry.backingStoreOffset > 0) {
        // Page exists in backing store, swap it in
        if (!swapIn(processId, pageNumber, frameNumber)) {
            freeFrames.push(frameNumber);
            return false;
        }
    }
    else {
        // New page, just mark it as present
        // In a real system, this would initialize the page with zeros
    }

    // Update page table entry
    entry.present = true;
    entry.referenced = true;
    entry.frameNumber = frameNumber;
    if (isWrite) {
        entry.dirty = true;
    }

    // Update frame information
    frames[frameNumber].occupied = true;
    frames[frameNumber].processId = processId;
    frames[frameNumber].pageNumber = pageNumber;
    frames[frameNumber].lastAccessed = std::chrono::steady_clock::now();

    usedFrames++;
    numPagedIn++;

    return true;
}

size_t DemandPagingAllocator::findVictimFrame() {
    // LRU (Least Recently Used) page replacement algorithm
    size_t victimFrame = 0;
    auto oldestTime = std::chrono::steady_clock::now();

    for (size_t i = 0; i < maxFrames; ++i) {
        if (frames[i].occupied && frames[i].lastAccessed < oldestTime) {
            oldestTime = frames[i].lastAccessed;
            victimFrame = i;
        }
    }

    return victimFrame;
}

bool DemandPagingAllocator::swapOut(size_t frameNumber) {
    if (frameNumber >= frames.size() || !frames[frameNumber].occupied) {
        return false;
    }

    FrameInfo& frame = frames[frameNumber];
    size_t processId = frame.processId;
    size_t pageNumber = frame.pageNumber;

    auto pageTableIt = processPageTables.find(processId);
    if (pageTableIt == processPageTables.end() || pageNumber >= pageTableIt->second.size()) {
        return false;
    }

    PageTableEntry& entry = pageTableIt->second[pageNumber];

    // Only write to backing store if page is dirty
    if (entry.dirty) {
        size_t backingStoreOffset = allocateBackingStoreSlot();

        // Write page data to backing store
        backingStore.seekp(backingStoreOffset * frameSize);
        std::vector<char> pageData(frameSize, 0); // Simulate page data
        backingStore.write(pageData.data(), frameSize);
        backingStore.flush();

        entry.backingStoreOffset = backingStoreOffset;
        backingStoreSize++;
    }

    // Update page table entry
    entry.present = false;
    entry.dirty = false;
    entry.referenced = false;

    // Free the frame
    frame.occupied = false;
    frame.processId = 0;
    frame.pageNumber = 0;
    usedFrames--;

    return true;
}

bool DemandPagingAllocator::swapIn(size_t processId, size_t pageNumber, size_t frameNumber) {
    auto pageTableIt = processPageTables.find(processId);
    if (pageTableIt == processPageTables.end() || pageNumber >= pageTableIt->second.size()) {
        return false;
    }

    PageTableEntry& entry = pageTableIt->second[pageNumber];

    if (entry.backingStoreOffset == 0) {
        return false; // Page not in backing store
    }

    // Read page data from backing store
    backingStore.seekg(entry.backingStoreOffset * frameSize);
    std::vector<char> pageData(frameSize);
    backingStore.read(pageData.data(), frameSize);

    if (!backingStore.good()) {
        return false;
    }

    // Free the backing store slot
    deallocateBackingStoreSlot(entry.backingStoreOffset);
    entry.backingStoreOffset = 0;
    backingStoreSize--;
    return true;
}

size_t DemandPagingAllocator::allocateBackingStoreSlot() {
    if (!freeBackingStoreSlots.empty()) {
        size_t slot = freeBackingStoreSlots.front();
        freeBackingStoreSlots.pop();
        return slot;
    }
    return nextBackingStoreOffset++;
}

void DemandPagingAllocator::deallocateBackingStoreSlot(size_t offset) {
    freeBackingStoreSlots.push(offset);
}

std::string DemandPagingAllocator::visualizeMemory() {
    std::lock_guard<std::mutex> lock(allocatorMutex);
    std::ostringstream oss;

    oss << "=== Demand Paging Memory Visualization ===\n";
    oss << "Physical Frames (" << maxFrames << " total):\n";

    for (size_t i = 0; i < maxFrames; ++i) {
        oss << "Frame " << i << ": ";
        if (frames[i].occupied) {
            oss << "Process " << frames[i].processId << ", Page " << frames[i].pageNumber;
        }
        else {
            oss << "Free";
        }
        oss << "\n";
    }

    oss << "\nProcess Page Tables:\n";
    for (const auto& [processId, pageTable] : processPageTables) {
        oss << "Process " << processId << ":\n";
        for (size_t pageNum = 0; pageNum < pageTable.size(); ++pageNum) {
            const PageTableEntry& entry = pageTable[pageNum];
            oss << "  Page " << pageNum << ": ";
            if (entry.present) {
                oss << "Frame " << entry.frameNumber;
                if (entry.dirty) oss << " (dirty)";
                if (entry.referenced) oss << " (ref)";
            }
            else if (entry.backingStoreOffset > 0) {
                oss << "Swapped (offset " << entry.backingStoreOffset << ")";
            }
            else {
                oss << "Not allocated";
            }
            oss << "\n";
        }
    }

    oss << "\nStatistics:\n";
    oss << "Page faults: " << numPageFaults.load() << "\n";
    oss << "Pages in memory: " << usedFrames.load() << "/" << maxFrames << "\n";
    oss << "Pages in backing store: " << backingStoreSize.load() << "\n";
    oss << "Total pages allocated: " << numPagedIn.load() << "\n";
    oss << "Total pages evicted: " << numPagedOut.load() << "\n";

    oss << "==========================================\n";

    return oss.str();
}

// Implement FlatMemoryAllocator methods that were missing
FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize, size_t memPerProcSize)
    : maxSize(maximumSize), memPerProc(memPerProcSize) {
    initializeMemory();
}

void* FlatMemoryAllocator::allocate(std::shared_ptr<Process> process) {
    size_t requiredSize = process->getMemoryRequired();

    // Find a free block of sufficient size
    for (size_t i = 0; i <= maxSize - requiredSize; i += memPerProc) {
        if (canAllocateAt(i, requiredSize)) {
            allocateAt(i, requiredSize);
            processMap[i] = process->getPID();
            return reinterpret_cast<void*>(i + 1); // Return non-zero pointer
        }
    }

    return nullptr; // No space available
}

void FlatMemoryAllocator::deallocate(std::shared_ptr<Process> process) {
    size_t processId = process->getPID();

    for (auto it = processMap.begin(); it != processMap.end(); ++it) {
        if (it->second == processId) {
            deallocateAt(it->first);
            processMap.erase(it);
            break;
        }
    }
}

std::string FlatMemoryAllocator::visualizeMemory() {
    std::ostringstream oss;
    oss << "Flat Memory Visualization:\n";

    for (size_t i = 0; i < maxSize; i += memPerProc) {
        auto procIt = processMap.find(i);
        if (procIt != processMap.end()) {
            oss << "Block " << i / memPerProc << " -> Process " << procIt->second << "\n";
        }
        else {
            oss << "Block " << i / memPerProc << " -> Free\n";
        }
    }

    return oss.str();
}

void FlatMemoryAllocator::initializeMemory() {
    memory.resize(maxSize, 0);

    for (size_t i = 0; i < maxSize; i += memPerProc) {
        allocationMap[i] = false;
    }
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
    size_t blocksNeeded = (size + memPerProc - 1) / memPerProc;

    for (size_t i = 0; i < blocksNeeded; ++i) {
        size_t blockIndex = index + (i * memPerProc);
        if (blockIndex >= maxSize || allocationMap.at(blockIndex)) {
            return false;
        }
    }

    return true;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
    size_t blocksNeeded = (size + memPerProc - 1) / memPerProc;

    for (size_t i = 0; i < blocksNeeded; ++i) {
        size_t blockIndex = index + (i * memPerProc);
        allocationMap[blockIndex] = true;
    }

    allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
    allocationMap[index] = false;
}
