#include "RRScheduler.h"

void RRScheduler::addProcess(std::shared_ptr<Process> process) {
    if (!running.load()) {
        return;  // Removed debug output
    }

    // Store process in processList for screen -ls functionality
    if (processList.size() <= static_cast<size_t>(process->getPID()))
        processList.resize(process->getPID() + 1);
    processList[process->getPID()] = process;

    // Try to allocate memory using the memory allocator
    void* memPtr = memoryAllocator.allocate(process);

    if (!memPtr) {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(process); // memory allocation failed → retry later
        return;
    }
    else {
        // Mark process as allocated 
        process->setAllocation(true);
    }

    std::lock_guard<std::mutex> lock(queueMutex);
    readyQueue.push(process);
    cvScheduler.notify_one();
}

void RRScheduler::schedulerThread() {
    while (running.load() || !readyQueue.empty()) {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Wait until there is a process ready to run or shutdown is requested
        cvScheduler.wait(lock, [this]() {
            return !readyQueue.empty() || !running.load();
            });

        // Assign the next process in the ready queue to an available CPU core
        static int lastAssignedCore = 0;
        for (int i = 0; i < numCores && !readyQueue.empty(); ++i) {
            int core = (lastAssignedCore + i) % numCores;
            if (!coreBusy[core].get()->load()) { // if the core is not busy
                std::shared_ptr<Process> proc = readyQueue.front();
                readyQueue.pop();

                if (!proc->isAllocated()) {
                    // Attempt to allocate memory for the process
                    void* memPtr = memoryAllocator.allocate(proc);

                    if (!memPtr) { // still no memory available
                        readyQueue.push(proc); // retry later
                        continue;
                    }
                    else {
                        proc->setAllocation(true);
                    }
                }

                {
                    std::lock_guard<std::mutex> core_lock(coreMutexes[core]);
                    currentProcess[core] = proc;
                    coreBusy[core].get()->store(true);
                }

                cvCores[core].notify_one(); // Notify the core thread to start processing
                lastAssignedCore = (core + 1) % numCores; // Round robin core assignment
            }
        }
    }
}

void RRScheduler::cpuCoreThread(int coreID) {
    while (running.load() || !readyQueue.empty() || currentProcess[coreID] != nullptr) {
        std::unique_lock<std::mutex> lock(coreMutexes[coreID]);

        cvCores[coreID].wait(lock, [this, coreID]() {
            return currentProcess[coreID] != nullptr || !running.load();
            });

        if (currentProcess[coreID] == nullptr) continue;

        std::shared_ptr<Process> proc = currentProcess[coreID];
        proc->setState(Process::State::RUNNING);

        if (proc->getStartTime() == std::chrono::system_clock::time_point{}) {
            proc->setStartTime(std::chrono::system_clock::now());
        }

        lock.unlock();

        // Execute process for quantum
        int instructionsExecuted = 0;
        while (instructionsExecuted < quantumCycles && !proc->isFinished()) {
            // Simulate page accesses during instruction execution
            DemandPagingAllocator* demandPagingAlloc = dynamic_cast<DemandPagingAllocator*>(&memoryAllocator);
            if (demandPagingAlloc && proc->getNumPages() > 0) {
                size_t pageToAccess = rand() % proc->getNumPages();
                bool isWrite = (rand() % 4 == 0); // 25% chance of write operation
                demandPagingAlloc->accessPage(proc->getPID(), pageToAccess, isWrite);
            }

            proc->executeCurrentCommand();
            proc->addLog(coreID, "Hello world from " + proc->getName());
            proc->moveToNextLine();
            instructionsExecuted++;

            if (delayPerExec > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExec));
            }
        }

        // Process completion handling
        lock.lock();
        if (proc->isFinished()) {
            proc->setState(Process::State::TERMINATED);
            proc->setEndTime(std::chrono::system_clock::now());

            // Deallocate memory for the process
            memoryAllocator.deallocate(proc);
            proc->setAllocation(false);

            // Ensure process is properly tracked
            if (processList.size() <= static_cast<size_t>(proc->getPID())) {
                processList.resize(proc->getPID() + 1);
            }
            processList[proc->getPID()] = proc;

            {
                std::lock_guard<std::mutex> finished_lock(finishedMutex);
                finishedProcesses.push_back(*proc);
            }

            currentProcess[coreID] = nullptr;
            coreBusy[coreID].get()->store(false);
            completedProcesses.fetch_add(1);

            // Explicitly notify in case this was the last process
            cvScheduler.notify_one();
        }
        else {
            proc->setState(Process::State::READY);
            {
                std::lock_guard<std::mutex> queue_lock(queueMutex);
                readyQueue.push(proc);
            }
            currentProcess[coreID] = nullptr;
            coreBusy[coreID].get()->store(false);
            cvScheduler.notify_one();
        }
    }
}