#include "AScheduler.h"

std::string Scheduler::getTimestamp() {
	char output[50];
	std::time_t timestamp = std::time(nullptr);
	struct tm dateTime;

	if (localtime_s(&dateTime, &timestamp) == 0) {
		strftime(output, 50, "%m/%d/%Y, %I:%M:%S%p", &dateTime);
		return std::string(output);
	}
	else {
		return "Error retrieving time";
	}
}

void Scheduler::writeToLog(const std::string& filename, const std::string& message) {
	std::ofstream logFile(filename, std::ios_base::app);
	if (logFile.is_open()) {
		logFile << message;
		logFile.close();
	}
	else {
		std::cerr << "Failed to open log file: " << filename << std::endl;
	}
}

void Scheduler::initializeLog(const Process& process) {
	std::ofstream logFile(process.logFilename, std::ios_base::trunc);
	if (logFile.is_open()) {
		logFile << "Process name: " << process.name << std::endl;
		logFile << "Logs: " << std::endl;
		logFile << std::endl;
		logFile.close();
	}
	else {
		std::cerr << "Failed to initialize log file: " << process.logFilename << std::endl;
	}
}

void Scheduler::displayScreenList() const {
	int activeCPUs = 0;
	for (int i = 0; i < numCores; ++i) {
		if (coreBusy[i].get()->load()) { activeCPUs++; }
	}
	int cpuUtil = activeCPUs * 100 / numCores;

	std::cout << "CPU utilization: " << cpuUtil << "%" << std::endl;
	std::cout << "Cores used: " << activeCPUs << std::endl;
	std::cout << "Cores available: " << numCores - activeCPUs << std::endl;
	std::cout << std::endl;

	std::cout << "--------------------------------" << std::endl;
	std::cout << "Running processes:" << std::endl;
	for (int i = 0; i < numCores; ++i) {
		Process* p = currentProcess[i];

		// TODO: Add timestamps when Process class is ready
		if (p != nullptr) {
			std::cout << p->name << "\tCore: " << i << "\t"
				<< p->currInst << " / " << p->instructionsRemaining << std::endl;
		}
	}
	std::cout << std::endl;

	std::cout << "Finished processes:" << std::endl;
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
		for (Process p : finishedProcesses) {
			std::cout << p.name << "\tFinished" << "\t"
				<< p.currInst << " / " << p.instructionsRemaining << std::endl;
		}
    }
	std::cout << "--------------------------------" << std::endl;
	std::cout << std::endl;
}

void Scheduler::schedulerStart() {
	// Start scheduler thread (detached)
	std::thread(&Scheduler::schedulerThread, this).detach();

	// Start CPU core threads (detached)
	for (int i = 0; i < numCores; ++i) {
		std::thread(&Scheduler::cpuCoreThread, this, i).detach();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Allow threads to start

	// Fake process creation logic
	for (int i = 0; i < TOTAL_PROCESSES; ++i) {
		std::string processName = "Process_" + std::to_string(i + 1);
		Process p(i + 1, processName);
		addProcess(p);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Main thread continues immediately; threads run in background
}

void Scheduler::schedulerStop() { /* no op */ }