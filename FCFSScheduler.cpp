#include "FCFSScheduler.h"

void FCFSScheduler::addProcess(std::shared_ptr<Process> process) {
	if (!running.load()) {
		std::cout << "[addProcess] Rejected " << process->getName() << " (scheduler is stopped)\n";
		return;
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
	while (running.load() || !readyQueue.empty()) {
		std::unique_lock<std::mutex> lock(coreMutexes[coreID]);

		// Wait until a process is assigned or shutdown is requested
		cvCores[coreID].wait(lock, [this, coreID]() {
			return currentProcess[coreID] != nullptr || !running.load();
			});


		if (currentProcess[coreID] != nullptr) {
			std::shared_ptr<Process> proc = currentProcess[coreID];
			proc->setState(Process::State::RUNNING);
			proc->setStartTime(std::chrono::system_clock::now());

			// OLD: Initialize process log file
			// initializeLog(*proc);

			lock.unlock(); // Unlock before processing to allow other cores to run

			// Simulate process execution
			for (int i = 0; i < proc->getCmdListSize(); ++i) {
				proc->executeCurrentCommand();
				// After proc->executeCurrentCommand();
				proc->addLog(coreID, "Hello world from " + proc->getName());
				proc->moveToNextLine();

				/* OLD BLOCK: log the command execution
				std::string timestamp = getTimestamp();
				std::stringstream logEntry;
				logEntry << "(" << timestamp << ") Core: " << coreID
					<< " \"" << output << "\"" << std::endl;
				 writeToLog(proc->logFilename, logEntry.str());
				*/
			}

			{ // safely add the process to the finished list
				proc->setState(Process::State::TERMINATED);
				std::lock_guard<std::mutex> finished_lock(finishedMutex);
				finishedProcesses.emplace_back(*proc);
			}

			// completion + cleanup
			lock.lock();

			currentProcess[coreID] = nullptr;
			coreBusy[coreID].get()->store(false); 
			completedProcesses++;
		}
		else {
			return;
		}
	}
}