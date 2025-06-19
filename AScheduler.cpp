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

std::ostringstream Scheduler::displayScreenList() const {
    std::ostringstream out;
    int activeCPUs = 0;
    for (int i = 0; i < numCores; ++i) {
        if (coreBusy[i].get()->load()) { activeCPUs++; }
    }
    int cpuUtil = activeCPUs * 100 / numCores;

    out << "CPU utilization: " << cpuUtil << "%" << std::endl;
    out << "Cores used: " << activeCPUs << std::endl;
    out << "Cores available: " << numCores - activeCPUs << std::endl;
    out << std::endl;

    out << "--------------------------------" << std::endl;
    out << "Running processes:\n";
    out << std::left
        << std::setw(16) << "Name"
        << std::setw(10) << "Core"
        << std::setw(14) << "Progress"
        << std::setw(12) << "StartTime"
        << "\n";

    for (int i = 0; i < numCores; ++i) {
        std::shared_ptr<Process> p = currentProcess[i];

        if (p == nullptr) { continue; }
        else if (p != nullptr) {
            std::time_t start_time_t = std::chrono::system_clock::to_time_t(p->getStartTime());
            struct tm localTime;

            if (localtime_s(&localTime, &start_time_t) == 0) {
                std::ostringstream timeStream;
                timeStream << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S %p");

                out << std::left << std::setw(12) << p->getName()
                    << "Core: " << i << "  "
                    << std::setw(5) << "\t" << p->getCounter()
                    << "\t" << timeStream.str()
                    << std::endl;
            }
            else {
                out << std::left << std::setw(12) << p->getName()
                    << "Core: " << i << "  "
                    << std::setw(5) << p->getCounter()
                    << "[Error getting time]"
                    << std::endl;
            }
        }
    }
    out << std::endl;

    out << "Finished processes:" << std::endl;
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
        for (const Process& p : finishedProcesses) {
            out << p.getName() << "\tFinished" << "\t"
                << p.getCounter() << std::endl;
        }
    }
    out << "--------------------------------" << std::endl;
    out << std::endl;
    return out;
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