#include "ProcessScheduler.h"

ProcessScheduler& ProcessScheduler::getInstance() {
	static ProcessScheduler instance;
	return instance;
}

void ProcessScheduler::init() {
	if (type == "fcfs") {
		scheduler = std::make_shared<FCFSScheduler>(numCPU);
	}
	else if (type == "rr") {
		scheduler = std::make_shared<RRScheduler>(numCPU, quantumCycles); //THIS IS EDITED
	}
	else {
		throw std::runtime_error("Unknown scheduler type: " + type);
	}

    scheduler->setBatchProcessFreq(batchProcessFreq);
    scheduler->setMinIns(minIns);
    scheduler->setMaxIns(maxIns);
    scheduler->setDelayPerExec(delayPerExec);
	
    // scheduler->schedulerStart();
    std::thread(&Scheduler::schedulerThread, scheduler).detach();
}

void ProcessScheduler::showProcessList() const {
    int max = 50;
    std::ostringstream out;
    for (const auto& p : scheduler->processList) {
		if (max <= 0) break; // Limit to max processes
		if (!p) continue; // Skip null pointers
		out << p->getName() << " " << p->getCounter() << std::endl;
        max--;
    }
    if (max <= 0) out << "... and " << scheduler->processList.size() - 50 << " more processes.";

    std::cout << "Process List: " << std::endl;
	std::cout << out.str() << std::endl;
}

void ProcessScheduler::showScreenList() const {
	std::cout << scheduler->displayScreenList().str();
}

void ProcessScheduler::makeReportUtil() const {
	std::string filename = "csopesy_log.txt";
	if (scheduler) {
		std::ofstream ofs(filename);
		if (!ofs) {
			std::cerr << "report-util error: Failed to open " << filename << std::endl;
			return;
		}
		ofs << scheduler->displayScreenList().str();
		ofs.close();
		std::cout << "Report successfully written to " << filename << std::endl;
    }
    else {
		std::cerr << "report-util warning: No scheduler initialized." << std::endl;
    }
}

std::shared_ptr<Process> ProcessScheduler::fetchProcessByName(const std::string& name) {
    bool procExists = false;

    if (scheduler) {
        for (const auto& process : scheduler->processList) {
            // Check if the process already exists
            if (process->getName() == name) {
                return std::make_shared<Process>(*process);
            }
        }
        if (!procExists) {
            // Process does not exist, create a new one
            int id = scheduler->processList.size() + 1;
            std::shared_ptr<Process> p = std::make_shared<Process>(id, name, this->minIns, this->maxIns);
            p->setState(Process::State::READY);
            // Set startTime to now for new process
            p->setStartTime(std::chrono::system_clock::now());
            scheduler->processList.push_back(p); // Add to list of procs
            scheduler->addProcess(p); // send to RQ
            return p;
        }
    }
	return nullptr;
}


void ProcessScheduler::loadConfigFromFile(const std::string& filename) {
    std::ifstream config("config.txt");
    if (!config) {
        std::cerr << "[WARN] Config file '" << filename << "' not found. Using default settings.\n";
        return;
    }

    std::string key;
    while (config >> key) {
        if (key == "num-cpu") {
            config >> numCPU;
        }
        else if (key == "scheduler") {
            config >> type;
        }
        else if (key == "quantum-cycles") {
            config >> quantumCycles;
        }
        else if (key == "batch-process-freq") {
            config >> batchProcessFreq;
        }
        else if (key == "min-ins") {
            config >> minIns;
        }
        else if (key == "max-ins") {
            config >> maxIns;
        }
        else if (key == "delay-per-exec") {
            config >> delayPerExec;
        }
        else {
            std::string unknownValue;
            config >> unknownValue; // discard
            std::cerr << "[WARN] Unknown config key: " << key << ", ignoring...\n";
        }
    }

    std::ostringstream out;
    out << "[INFO] Loaded config from " << filename << ":\n"
        << "  num-cpu: " << numCPU << "\n"
        << "  scheduler: " << type << "\n"
        << "  quantum-cycles: " << quantumCycles << "\n"
        << "  batch-process-freq: " << batchProcessFreq << "\n"
        << "  min-ins: " << minIns << "\n"
        << "  max-ins: " << maxIns << "\n"
        << "  delay-per-exec: " << delayPerExec << "\n";
}

void ProcessScheduler::start() {
    if (scheduler) scheduler->schedulerStart();
}

void ProcessScheduler::stop() {
    if (scheduler) scheduler->schedulerStop();
}
