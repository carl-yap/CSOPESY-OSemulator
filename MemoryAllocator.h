#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <filesystem>
#include <queue>
#include <fstream>
#include <atomic>
#include <mutex>

#include "Process.h"

// Enhanced interface with demand paging support
class IMemoryAllocator {
public:
    virtual void* allocate(std::shared_ptr<Process> process) = 0;
    virtual void deallocate(std::shared_ptr<Process> process) = 0;
    virtual std::string visualizeMemory() = 0;
    virtual ~IMemoryAllocator() = default;
};

// Page table entry structure
struct PageTableEntry {
    bool present = false;           // Is page in physical memory?
    bool dirty = false;             // Has page been modified?
    bool referenced = false;        // Has page been accessed recently?
    size_t frameNumber = 0;         // Physical frame number if present
    size_t backingStoreOffset = 0;  // Offset in backing store if swapped out

    PageTableEntry() = default;
};

// Frame information for physical memory management
struct FrameInfo {
    bool occupied = false;
    size_t processId = 0;
    size_t pageNumber = 0;
    std::chrono::steady_clock::time_point lastAccessed;

    FrameInfo() : lastAccessed(std::chrono::steady_clock::now()) {}
};

class DemandPagingAllocator : public IMemoryAllocator {
public:
    DemandPagingAllocator(size_t maxMemorySize, size_t frameSize, size_t maxFrames, const std::string& backingStoreFile = "backing_store.dat");
    ~DemandPagingAllocator();

    void* allocate(std::shared_ptr<Process> process) override;
    void deallocate(std::shared_ptr<Process> process) override;
    std::string visualizeMemory() override;

    // Page access simulation (called when process tries to access memory)
    bool accessPage(size_t processId, size_t pageNumber, bool isWrite = false);

    // Statistics for vmstat
    size_t getUsedFrames() const { return usedFrames.load(); }
    size_t getTotalFrames() const { return maxFrames; }
    size_t getFreeFrames() const { return maxFrames - usedFrames.load(); }
    size_t getNumPagedIn() const { return numPagedIn.load(); }
    size_t getNumPagedOut() const { return numPagedOut.load(); }
    size_t getNumPageFaults() const { return numPageFaults.load(); }
    size_t getBackingStoreSize() const { return backingStoreSize.load(); }

private:
    size_t maxMemorySize;
    size_t frameSize;
    size_t maxFrames;
    std::string backingStoreFile;

    // Physical memory frames
    std::vector<FrameInfo> frames;
    std::queue<size_t> freeFrames;

    // Process page tables
    std::unordered_map<size_t, std::vector<PageTableEntry>> processPageTables;

    // Backing store management
    std::fstream backingStore;
    std::queue<size_t> freeBackingStoreSlots;
    size_t nextBackingStoreOffset;

    // Statistics (thread-safe)
    mutable std::atomic<size_t> usedFrames{ 0 };
    mutable std::atomic<size_t> numPagedIn{ 0 };
    mutable std::atomic<size_t> numPagedOut{ 0 };
    mutable std::atomic<size_t> numPageFaults{ 0 };
    mutable std::atomic<size_t> backingStoreSize{ 0 };

    // Thread safety
    mutable std::mutex allocatorMutex;

    // Helper methods
    size_t findVictimFrame();           // LRU page replacement
    bool swapOut(size_t frameNumber);   // Swap page to backing store
    bool swapIn(size_t processId, size_t pageNumber, size_t frameNumber); // Swap page from backing store
    void initializeBackingStore();
    size_t allocateBackingStoreSlot();
    void deallocateBackingStoreSlot(size_t offset);
};

// Keep existing allocators for backward compatibility
class FlatMemoryAllocator : public IMemoryAllocator {
public:
    FlatMemoryAllocator(size_t maximumSize, size_t memPerProcSize);
    ~FlatMemoryAllocator() { memory.clear(); }

    void* allocate(std::shared_ptr<Process> process) override;
    void deallocate(std::shared_ptr<Process> process) override;
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

    // Methods for vmstat
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

    mutable std::atomic<size_t> numPagedIn{ 0 };
    mutable std::atomic<size_t> numPagedOut{ 0 };

    size_t allocateFrames(size_t numFrames, size_t processID);
    void deallocateFrames(size_t numFrames, size_t frameIndex);
};