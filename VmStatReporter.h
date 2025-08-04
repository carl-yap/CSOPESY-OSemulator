#pragma once
#include <iostream>
#include <string>

class VmStatReporter {
public:
    static VmStatReporter& getInstance();

    void setMaxMemory(size_t total);
    void setUsedMemory(size_t used);
    void incrementIdleTicks();
    void incrementActiveTicks();
    void incrementPageIn();
    void incrementPageOut();

    void displayStats() const;

private:
    VmStatReporter() = default;

    size_t totalMemory = 0;
    size_t usedMemory = 0;

    uint64_t idleCpuTicks = 0;
    uint64_t activeCpuTicks = 0;

    uint64_t pagedIn = 0;
    uint64_t pagedOut = 0;
};
