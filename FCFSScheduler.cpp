#include "FCFSScheduler.h"

void FCFSScheduler::addProcess(std::shared_ptr<Process> process) {
	if (!running.load()) {
		std::cout << "[addProcess] Rejected " << process->getName() << " (scheduler is stopped)\n";
		return;
	}

	// Store process in processList for screen -ls functionality
	if (processList.size() <= static_cast<size_t>(process->getPID()))
		processList.resize(process->getPID() + 1);
	processList[process->getPID()] = process;

	// SET PID before allocating
	//static_cast<FlatMemoryAllocator&>(memoryAllocator).setCurrentPID(process->getPID());
	//void* memPtr = memoryAllocator.allocate(memPerProc);
	static_cast<PagingAllocator&>(memoryAllocator);
	void* memPtr = memoryAllocator.allocate(process);

	if (!memPtr) {
		//std::cout << "[addProcess] Memory full for " << process->getName() << ", moving to back of queue\n";
		std::lock_guard<std::mutex> lock(queueMutex);
		readyQueue.push(process); // memory full → retry later
		return;
	}
	else { // mark process as allocated 
		process->setAllocation(true);
		//process->setAllocationIndex(memPtr);
	}

	std::lock_guard<std::mutex> lock(queueMutex);
	readyQueue.push(process);
	cvScheduler.notify_one();
}

void FCFSScheduler::schedulerThread() {
	while (running.load() || !readyQueue.empty()) {
		std::unique_lock<std::mutex> lock(queueMutex);

		// Wait until there is a process ready to run or shutdown is requested
		cvScheduler.wait(lock, [this]() {
			return !readyQueue.empty() || !running.load();
			});

		// Assign the next process in the RQ to a CPU core
		static int lastAssignedCore = 0;
		for (int i = 0; i < numCores && !readyQueue.empty(); ++i) {
			int core = (lastAssignedCore + i) % numCores;
			if (!coreBusy[core].get()->load()) { // if the core is not busy
				std::shared_ptr<Process> proc = readyQueue.front();
				readyQueue.pop();

				if (!proc->isAllocated()) {
					// Attempt to allocate memory for the process
					static_cast<PagingAllocator&>(memoryAllocator);
					void* memPtr = memoryAllocator.allocate(proc);

					if (!memPtr) { // still no memory available
						readyQueue.push(proc); // retry later
						continue;
					}
					else { // it found a block open
						proc->setAllocation(true);
						//proc->setAllocationIndex(memPtr);
					}
				}

				{
					std::lock_guard<std::mutex> core_lock(coreMutexes[core]);
					currentProcess[core] = proc;
					coreBusy[core].get()->store(true);
				}

				cvCores[core].notify_one(); // Notify the core thread to start processing
			}
		}
	}
}

void FCFSScheduler::cpuCoreThread(int coreID) {
	while (running.load() || !readyQueue.empty() || currentProcess[coreID] != nullptr) {
		std::unique_lock<std::mutex> lock(coreMutexes[coreID]);

		// Wait until a process is assigned or shutdown is requested
		cvCores[coreID].wait(lock, [this, coreID]() {
			return currentProcess[coreID] != nullptr || !running.load();
			});

		if (currentProcess[coreID] == nullptr) continue;

		std::shared_ptr<Process> proc = currentProcess[coreID];
		proc->setState(Process::State::RUNNING);

		if (proc->getStartTime() == std::chrono::system_clock::time_point{}) {
			proc->setStartTime(std::chrono::system_clock::now());
		}

		lock.unlock(); // Unlock before processing to allow other cores to run

		// Execute process until completion (FCFS doesn't preempt)
		int instructionsExecuted = 0;
		while (!proc->isFinished()) {
			proc->executeCurrentCommand();
			// After proc->executeCurrentCommand();
			proc->addLog(coreID, "Hello world from " + proc->getName());
			proc->moveToNextLine();
			instructionsExecuted++;

			if (delayPerExec > 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(delayPerExec));
			}
		}

		// Process completion handling
		lock.lock();
		proc->setState(Process::State::TERMINATED);
		proc->setEndTime(std::chrono::system_clock::now());

		// Deallocate memory for the process
		static_cast<PagingAllocator&>(memoryAllocator).deallocate(proc);
		//proc->setAllocationIndex(nullptr);
		proc->setAllocation(false);

		// Ensure process is properly tracked
		if (processList.size() <= static_cast<size_t>(proc->getPID())) {
			processList.resize(proc->getPID() + 1);
		}
		processList[proc->getPID()] = proc;

		{ // safely add the process to the finished list
			std::lock_guard<std::mutex> finished_lock(finishedMutex);
			finishedProcesses.emplace_back(*proc);
		}

		// completion + cleanup
		currentProcess[coreID] = nullptr;
		coreBusy[coreID].get()->store(false);
		completedProcesses.fetch_add(1);

		// Explicitly notify in case this was the last process
		cvScheduler.notify_one();
	}
}