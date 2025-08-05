#include "DemandPaging.h"
#include <cassert>

// PhysicalMemory implementation
PhysicalMemory::PhysicalMemory() : frames(NUM_FRAMES, std::vector<uint8_t>(PAGE_SIZE, 0)) {
    for (size_t i = 0; i < NUM_FRAMES; ++i) freeFrames.push(i);
}

std::optional<size_t> PhysicalMemory::allocateFrame() {
    if (!freeFrames.empty()) {
        size_t f = freeFrames.front();
        freeFrames.pop();
        fifoQueue.push(f);
        return f;
    }
    return std::nullopt;
}

size_t PhysicalMemory::evictFrame() {
    assert(!fifoQueue.empty());
    size_t f = fifoQueue.front();
    fifoQueue.pop();
    fifoQueue.push(f); // Re-add for round-robin
    return f;
}

void PhysicalMemory::freeFrame(size_t f) {
    freeFrames.push(f);
}

// BackingStore implementation
BackingStore::BackingStore() : pages(NUM_PAGES, std::vector<uint8_t>(PAGE_SIZE, 0)) {}

// Process implementation
Process::Process() : pageTable(NUM_PAGES) {}

// DemandPager implementation
DemandPager::DemandPager() : frameTable(NUM_FRAMES, {nullptr, 0}) {}

uint8_t DemandPager::read(Process& proc, size_t vaddr) {
    size_t page = vaddr / PAGE_SIZE;
    size_t offset = vaddr % PAGE_SIZE;
    ensurePageInMemory(proc, page);
    size_t frame = proc.pageTable[page].frame;
    return physMem.frames[frame][offset];
}

void DemandPager::write(Process& proc, size_t vaddr, uint8_t value) {
    size_t page = vaddr / PAGE_SIZE;
    size_t offset = vaddr % PAGE_SIZE;
    ensurePageInMemory(proc, page);
    size_t frame = proc.pageTable[page].frame;
    physMem.frames[frame][offset] = value;
    proc.pageTable[page].dirty = true;
}

void DemandPager::ensurePageInMemory(Process& proc, size_t page) {
    if (proc.pageTable[page].present) return;

    // Try to allocate a free frame
    auto frameOpt = physMem.allocateFrame();
    size_t frame;
    if (frameOpt) {
        frame = *frameOpt;
    } else {
        // Need to evict a page
        frame = physMem.evictFrame();
        auto& [victimProc, victimPage] = frameTable[frame];
        if (victimProc && victimProc->pageTable[victimPage].dirty) {
            // Write back to backing store
            victimProc->backingStore.pages[victimPage] = physMem.frames[frame];
        }
        if (victimProc) {
            victimProc->pageTable[victimPage].present = false;
            victimProc->pageTable[victimPage].dirty = false;
        }
    }

    // Load page from backing store
    physMem.frames[frame] = proc.backingStore.pages[page];
    proc.pageTable[page].present = true;
    proc.pageTable[page].frame = frame;
    proc.pageTable[page].dirty = false;
    frameTable[frame] = {&proc, page};
    std::cout << "Page fault: loaded page " << page << " into frame " << frame << "\n";
}