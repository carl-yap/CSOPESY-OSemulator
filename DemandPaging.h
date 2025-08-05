#pragma once
#include <vector>
#include <queue>
#include <cstdint>
#include <optional>
#include <iostream>

// Constants for paging
constexpr size_t PAGE_SIZE = 256;
constexpr size_t NUM_PAGES = 16;
constexpr size_t NUM_FRAMES = 4;

// Page Table Entry
struct PageTableEntry {
    bool present = false;
    size_t frame = 0;
    bool dirty = false;
};

// Simulated Physical Memory
class PhysicalMemory {
public:
    PhysicalMemory();
    std::optional<size_t> allocateFrame();
    size_t evictFrame();
    void freeFrame(size_t f);
    std::vector<std::vector<uint8_t>> frames;
private:
    std::queue<size_t> freeFrames;
    std::queue<size_t> fifoQueue;
};

// Simulated Backing Store (per process)
struct BackingStore {
    std::vector<std::vector<uint8_t>> pages;
    BackingStore();
};

// Process
struct Process {
    std::vector<PageTableEntry> pageTable;
    BackingStore backingStore;
    Process();
};

// Demand Paging System
class DemandPager {
public:
    DemandPager();
    uint8_t read(Process& proc, size_t vaddr);
    void write(Process& proc, size_t vaddr, uint8_t value);
    PhysicalMemory physMem;
private:
    void ensurePageInMemory(Process& proc, size_t page);
    std::vector<std::pair<Process*, size_t>> frameTable;
};