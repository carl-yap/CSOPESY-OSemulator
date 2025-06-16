#include "FCFSScheduler.h"

void FCFSScheduler::addProcess(const Process& process) {
	std::lock_guard<std::mutex> lock(queueMutex);
	readyQueue.push(process);
	cvScheduler.notify_one(); // Notify the scheduler that a process was pushed to RQ
}

void FCFSScheduler::schedulerThread() {
	while (running.load() || !readyQueue.empty()) {
		std::unique_lock<std::mutex> lock(queueMutex);

		// Wait until there is a process ready to run or shutdown is requested
		cvScheduler.wait(lock, [this]() {
			return !readyQueue.empty() || !running.load();
			});

		// Assign the next process in the RQ to a CPU core
		for (int core = 0; core < numCores && !readyQueue.empty(); ++core) {
			if (!coreBusy[core].get()->load()) { // if the core is not busy
				Process proc = readyQueue.front();
				readyQueue.pop();

				{
					std::lock_guard<std::mutex> core_lock(coreMutexes[core]);
					currentProcess[core] = new Process(proc);
					coreBusy[core].get()->store(true);
				}

				cvCores[core].notify_one(); // Notify the core thread to start processing
			}
		}
	}
}

void FCFSScheduler::cpuCoreThread(int coreID) {
	while (running.load() || !readyQueue.empty()) {
		std::unique_lock<std::mutex> lock(coreMutexes[coreID]);

		// Wait until a process is assigned or shutdown is requested
		cvCores[coreID].wait(lock, [this, coreID]() {
			return currentProcess[coreID] != nullptr || !running.load();
			});

		if (currentProcess[coreID] != nullptr) {
			Process* proc = currentProcess[coreID];

			// Initialize process log file
			initializeLog(*proc);

			lock.unlock(); // Unlock before processing to allow other cores to run

			// Simulate process execution
			for (int i = 0; i < proc->instructionsRemaining; ++i) {
				std::string output = proc->executeInstruction();

				// log the command execution
				std::string timestamp = getTimestamp();
				std::stringstream logEntry;
				logEntry << "(" << timestamp << ") Core: " << coreID
					<< " \"" << output << "\"" << std::endl;
				 writeToLog(proc->logFilename, logEntry.str());
			}

			{ // safely add the process to the finished list
				std::lock_guard<std::mutex> finished_lock(finishedMutex);
				finishedProcesses.emplace_back(*proc);
			}

			// completion + cleanup
			lock.lock();

			delete currentProcess[coreID]; // Free the process memory
			currentProcess[coreID] = nullptr;
			coreBusy[coreID].get()->store(false); 
			completedProcesses++;

			// if all processes are completed, notify the scheduler
			if (completedProcesses.load() >= TOTAL_PROCESSES) {
				running.store(false);
				cvScheduler.notify_all(); // Notify the scheduler to stop
				for (int i = 0; i < numCores; ++i) {
					cvCores[i].notify_all(); // Notify all cores to stop
				}
			}
		}
	}
}