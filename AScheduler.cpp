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

/* =============OLD FUNCTIONS TO CREATE LOG FILES=================
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
*/

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
	std::cout << "Running processes:\n";
	std::cout << std::left
		<< std::setw(16) << "Name"
		<< std::setw(10) << "Core"
		<< std::setw(14) << "Progress"
		<< std::setw(12) << "StartTime"
		<< "\n";

	for (int i = 0; i < numCores; ++i) {
		std::shared_ptr<Process> p = currentProcess[i];

		// TODO: Add timestamps when Process class is ready
		if (p == nullptr) { continue; }
		else if (p != nullptr) {
			std::time_t start_time_t = std::chrono::system_clock::to_time_t(p->getStartTime());
			struct tm localTime;

			if (localtime_s(&localTime, &start_time_t) == 0) {
				std::ostringstream timeStream;
				timeStream << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S %p");

				std::cout << std::left << std::setw(12) << p->getName()
					<< "Core: " << i << "  "
					<< std::setw(5) << "\t" << p->getCounter()
					<< "\t" << timeStream.str()
					<< std::endl;
			}
			else {
				std::cout << std::left << std::setw(12) << p->getName()
					<< "Core: " << i << "  "
					<< std::setw(5) << p->getCounter()
					<< "[Error getting time]"
					<< std::endl;
			}
		}
	}
	std::cout << std::endl;

	std::cout << "Finished processes:" << std::endl;
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
		for (Process p : finishedProcesses) {
			std::cout << p.getName() << "\tFinished" << "\t"
				<< p.getCounter() << std::endl;
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
		std::shared_ptr<Process> p = std::make_shared<Process>(i + 1, processName, 100, 500);
		p->setState(Process::State::READY);
		this->processList[i] = p; // temp storage for ProcessConsole
		addProcess(p);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Main thread continues immediately; threads run in background
}

void Scheduler::schedulerStop() { /* no op */ }