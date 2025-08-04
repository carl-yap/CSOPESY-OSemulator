#include "VmStatReporter.h"

VmStatReporter& VmStatReporter::getInstance() {
    static VmStatReporter instance;
    return instance;
}

void VmStatReporter::setMaxMemory(size_t total) { totalMemory = total; }
void VmStatReporter::setUsedMemory(size_t used) { usedMemory = used; }
void VmStatReporter::incrementIdleTicks() { idleCpuTicks++; }
void VmStatReporter::incrementActiveTicks() { activeCpuTicks++; }
void VmStatReporter::incrementPageIn() { pagedIn++; }
void VmStatReporter::incrementPageOut() { pagedOut++; }

void VmStatReporter::displayStats() const {
    std::cout << "=== vmstat ===" << std::endl;
    std::cout << "Total memory:        " << totalMemory << " bytes" << std::endl;
    std::cout << "Used memory:         " << usedMemory << " bytes" << std::endl;
    std::cout << "Free memory:         " << (totalMemory - usedMemory) << " bytes" << std::endl;
    std::cout << "Idle CPU ticks:      " << idleCpuTicks << std::endl;
    std::cout << "Active CPU ticks:    " << activeCpuTicks << std::endl;
    std::cout << "Total CPU ticks:     " << (idleCpuTicks + activeCpuTicks) << std::endl;
    std::cout << "Pages Paged In:      " << pagedIn << std::endl;
    std::cout << "Pages Paged Out:     " << pagedOut << std::endl;
}
