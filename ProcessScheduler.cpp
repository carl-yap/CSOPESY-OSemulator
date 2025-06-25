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
	scheduler->schedulerStart();

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
	if (scheduler) {
		for (const auto& process : scheduler->processList) {
			if (process->getName() == name) {
				return std::make_shared<Process>(*process);
			}
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

    std::cout << "[INFO] Loaded config from " << filename << ":\n"
        << "  num-cpu: " << numCPU << "\n"
        << "  scheduler: " << type << "\n"
        << "  quantum-cycles: " << quantumCycles << "\n"
        << "  batch-process-freq: " << batchProcessFreq << "\n"
        << "  min-ins: " << minIns << "\n"
        << "  max-ins: " << maxIns << "\n"
        << "  delay-per-exec: " << delayPerExec << "\n";
}

void ProcessScheduler::stop() {
    if (scheduler) scheduler->schedulerStop();
}
