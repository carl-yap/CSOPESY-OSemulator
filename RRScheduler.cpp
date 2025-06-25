#include "RRScheduler.h"

void RRScheduler::addProcess(std::shared_ptr<Process> process) {
    if (!running.load()) {
        std::cout << "[addProcess] Rejected " << process->getName() << " (scheduler is stopped)\n";
        return;
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
    while (running.load() || !readyQueue.empty()) {
        std::unique_lock<std::mutex> lock(coreMutexes[coreID]);

        // Wait until a process is assigned or shutdown is requested
        cvCores[coreID].wait(lock, [this, coreID]() {
            return currentProcess[coreID] != nullptr || !running.load();
            });

        if (currentProcess[coreID] != nullptr) {
            std::shared_ptr<Process> proc = currentProcess[coreID];
            proc->setState(Process::State::RUNNING);

            // Set start time only if this is the first time running
            if (proc->getStartTime() == std::chrono::system_clock::time_point{}) {
                proc->setStartTime(std::chrono::system_clock::now());
            }

            lock.unlock(); // Unlock before processing to allow other cores to run

            // Execute process for quantum cycles or until completion
            int instructionsExecuted = 0;
            while (instructionsExecuted < quantumCycles && !proc->isFinished()) {
                proc->executeCurrentCommand();
                proc->moveToNextLine();
                instructionsExecuted++;

                // Apply delay if configured
                if (delayPerExec > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExec));
                }
            }

            // Check if process is finished
            if (proc->isFinished()) {
                // Process completed
                proc->setState(Process::State::TERMINATED);
                proc->setEndTime(std::chrono::system_clock::now());

                {
                    std::lock_guard<std::mutex> finished_lock(finishedMutex);
                    finishedProcesses.emplace_back(*proc);
                }

                // Cleanup
                lock.lock();
                currentProcess[coreID] = nullptr;
                coreBusy[coreID].get()->store(false);
                completedProcesses++;
            }
            else {
                // Process not finished - preempt and put back in ready queue
                proc->setState(Process::State::READY);

                {
                    std::lock_guard<std::mutex> queue_lock(queueMutex);
                    readyQueue.push(proc);
                    cvScheduler.notify_one();
                }

                // Cleanup
                lock.lock();
                currentProcess[coreID] = nullptr;
                coreBusy[coreID].get()->store(false);
            }
        }
        else {
            return;
        }
    }
}