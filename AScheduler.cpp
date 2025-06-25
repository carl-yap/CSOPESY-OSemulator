#include "AScheduler.h"
#include "ProcessScheduler.h"

inline static std::atomic_int globalProcessCounter{ 1 };

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

    // Prepare running process info
    std::vector<std::string> runningInfo;

    for (int i = 0; i < numCores; ++i) {
        std::lock_guard<std::mutex> lock(coreMutexes[i]);

        if (coreBusy[i] && coreBusy[i]->load()) {
            activeCPUs++;

            std::shared_ptr<Process> p = currentProcess[i];
            if (p) {
                std::time_t start_time_t = std::chrono::system_clock::to_time_t(p->getStartTime());
                struct tm localTime;

                std::ostringstream timeStream;
                if (localtime_s(&localTime, &start_time_t) == 0) {
                    timeStream << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S %p");
                }
                else {
                    timeStream << "[time error]";
                }

                std::ostringstream line;
                line << std::left << std::setw(16) << p->getName()
                    << std::setw(10) << ("Core " + std::to_string(i))
                    << std::setw(14) << p->getCounter()
                    << std::setw(12) << timeStream.str();

                runningInfo.push_back(line.str());
            }
        }
    }

    int cpuUtil = (numCores > 0) ? (activeCPUs * 100 / numCores) : 0;

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

    for (const auto& line : runningInfo) {
        out << line << std::endl;
    }

    out << std::endl;

    out << "Finished processes:" << std::endl;
    {
        std::lock_guard<std::mutex> lock(finishedMutex);
        for (const Process& p : finishedProcesses) {
            out << std::left << std::setw(16) << p.getName()
                << "Finished\t" << p.getCounter() << std::endl;
        }
    }

    out << "--------------------------------" << std::endl;
    out << std::endl;
    return out;
}


void Scheduler::schedulerStart() {
    
    running = true;
    startTickThread();

    // Start scheduler and cores
	// std::thread(&Scheduler::schedulerThread, this).detach(); MOVED THIS TO ProcessScheduler.init()
    for (int i = 0; i < numCores; ++i)
        std::thread(&Scheduler::cpuCoreThread, this, i).detach();

    // Preload initial processes immediately (use unique ID range)
    for (int i = 0; i < numCores * 2; ++i) {
        int pid = globalProcessCounter.fetch_add(1); // fetch next global number
        std::string processName = "p" + std::to_string(pid);

        std::shared_ptr<Process> p = std::make_shared<Process>(pid, processName, minIns, maxIns);
        p->setState(Process::State::READY);
        addProcess(p);
    }

    // Start batch process generation thread
    std::thread([this]() {
        while (running.load()) {
            uint64_t lastTick = tickCount.load();

            while (tickCount.load() < lastTick + batchProcessFreq && running.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (!running.load()) {
                break;
            }

            int pid = globalProcessCounter.fetch_add(1); // continue where preload left off
            std::string processName = "p" + std::to_string(pid);

            std::shared_ptr<Process> p = std::make_shared<Process>(
                pid, processName, minIns, maxIns
            );
            p->setState(Process::State::READY);

            if (processList.size() <= static_cast<size_t>(pid))
                processList.resize(pid + 1);

            processList[pid] = p;
            addProcess(p);
        }
        }).detach();
}


void Scheduler::schedulerStop() {
    running.store(false);
    tickThreadRunning.store(false);

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        while (!readyQueue.empty()) readyQueue.pop();
    }

    cvScheduler.notify_all();
    for (auto& cv : cvCores)
        cv.notify_all();
}


void Scheduler::startTickThread() {
    if (tickThreadRunning.load()) return;  // Prevent multiple tick threads
    tickThreadRunning = true;

    std::thread([]() {
        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            tickCount++;
        }
        }).detach();
}
