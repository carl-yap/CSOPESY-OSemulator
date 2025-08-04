#include "ProcessScheduler.h"

ProcessScheduler& ProcessScheduler::getInstance() {
	static ProcessScheduler instance;
	return instance;
}

void ProcessScheduler::init() {
    // MCO2 requirement
	size_t numFrames = maxOverallMem / memPerFrame;
    memoryAllocator = std::make_shared<PagingAllocator>(maxOverallMem, numFrames);

	if (type == "fcfs") {
        scheduler = std::make_shared<FCFSScheduler>(numCPU, *memoryAllocator);
	}
	else if (type == "rr") {
        scheduler = std::make_shared<RRScheduler>(numCPU, quantumCycles, *memoryAllocator);
	}
	else {
		throw std::runtime_error("Unknown scheduler type: " + type);
	}

    // copying scheduler variables
    scheduler->setBatchProcessFreq(batchProcessFreq);
    scheduler->setMinIns(minIns);
    scheduler->setMaxIns(maxIns);
    scheduler->setDelayPerExec(delayPerExec);
    scheduler->setMaxOverallMemory(static_cast<int>(maxOverallMem));
	scheduler->setMemPerFrame(memPerFrame);
	scheduler->setMinMemPerProc(minMemPerProc);
	scheduler->setMaxMemPerProc(maxMemPerProc);
	
    // scheduler->schedulerStart();
    std::thread(&Scheduler::schedulerThread, scheduler).detach();
    for (int i = 0; i < numCPU; ++i) {
        std::thread(&Scheduler::cpuCoreThread, scheduler, i).detach();
    }
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

std::shared_ptr<Process> ProcessScheduler::fetchProcessByName(const std::string& name, size_t memSize) {
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
            int id = static_cast<int>(scheduler->processList.size()) + 1;
			size_t requiredMem = memSize > 0 ? memSize : minMemPerProc + rand() % (maxMemPerProc - minMemPerProc + 1);
			size_t numPages = requiredMem / this->memPerFrame;
            std::shared_ptr<Process> p = std::make_shared<Process>(id, name, this->minIns, this->maxIns, requiredMem, numPages);
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
        else if (key == "max-overall-mem") {
            config >> maxOverallMem;
        }
        else if (key == "mem-per-frame") {
			config >> memPerFrame;
        }
        else if (key == "min-mem-per-proc") {
            config >> minMemPerProc;
        }
        else if (key == "max-mem-per-proc") {
			config >> maxMemPerProc;
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
        << "  delay-per-exec: " << delayPerExec << "\n"
		<< "  max-overall-mem: " << maxOverallMem << "\n"
		<< "  mem-per-frame: " << memPerFrame << "\n"
        << "  min-mem-per-proc: " << minMemPerProc << "\n"
        << "  max-mem-per-proc: " << maxMemPerProc << "\n";
}

void ProcessScheduler::start() {
    if (scheduler) scheduler->schedulerStart();
}

void ProcessScheduler::stop() {
    if (scheduler) {
        scheduler->schedulerStop();  // signal all threads to stop
        // scheduler = nullptr;         // force cleanup if shared_ptr
    }
}

void ProcessScheduler::exit() {
    if (scheduler) scheduler->cleanUp();
}

bool ProcessScheduler::isValidMemorySize(size_t size) const {
    return size <= maxOverallMem && size <= maxMemPerProc;
}

void ProcessScheduler::showVMStat() const {
    if (!scheduler || !memoryAllocator) {
        std::cerr << "vmstat error: No scheduler or memory allocator initialized." << std::endl;
        return;
    }

    // Calculate memory statistics
    size_t totalMemory = maxOverallMem;
    size_t usedMemory = 0;
    size_t freeMemory = totalMemory;
    
    // Count active processes and calculate used memory
    int activeProcesses = 0;
    int inactiveProcesses = 0;
    
    for (const auto& process : scheduler->processList) {
        if (!process) continue;
        
        if (process->getState() == Process::State::RUNNING || 
            process->getState() == Process::State::READY ||
            process->getState() == Process::State::WAITING) {
            activeProcesses++;
            if (process->isAllocated()) {
                usedMemory += process->getMemoryRequired();
            }
        } else {
            inactiveProcesses++;
        }
    }
    
    freeMemory = totalMemory - usedMemory;
    
    // Calculate CPU tick statistics
    uint64_t totalCpuTicks = scheduler->getTickCount() * numCPU;
    uint64_t activeCpuTicks = 0;
    uint64_t idleCpuTicks = 0;
    
    // Count active CPU cores
    int activeCores = 0;
    const auto& coreBusyFlags = scheduler->getCoreBusy();
    for (int i = 0; i < numCPU; ++i) {
        if (i < static_cast<int>(coreBusyFlags.size()) && 
            coreBusyFlags[i] && 
            coreBusyFlags[i]->load()) {
            activeCores++;
        }
    }
    
    // Estimate active vs idle ticks (simplified calculation)
    activeCpuTicks = scheduler->getTickCount() * activeCores;
    idleCpuTicks = totalCpuTicks - activeCpuTicks;
    
    // Placeholder values for paging (as requested)
    size_t numPagedIn = 0;   // Placeholder
    size_t numPagedOut = 0;  // Placeholder
    
    // Display vmstat information
    std::cout << "=== VMSTAT ===" << std::endl;
    std::cout << std::endl;
    
    // Process information
    std::cout << "Active processes: " << activeProcesses << std::endl;
    std::cout << "Inactive processes: " << inactiveProcesses << std::endl;
    std::cout << std::endl;
    
    // Memory information  
    std::cout << "Total memory: " << totalMemory << " bytes" << std::endl;
    std::cout << "Used memory: " << usedMemory << " bytes" << std::endl;
    std::cout << "Free memory: " << freeMemory << " bytes" << std::endl;
    std::cout << std::endl;
    
    // CPU tick information
    std::cout << "Idle cpu ticks: " << idleCpuTicks << std::endl;
    std::cout << "Active cpu ticks: " << activeCpuTicks << std::endl; 
    std::cout << "Total cpu ticks: " << totalCpuTicks << std::endl;
    std::cout << std::endl;
    
    // Paging information (placeholder values)
    std::cout << "Num paged in: " << numPagedIn << std::endl;
    std::cout << "Num paged out: " << numPagedOut << std::endl;
    std::cout << std::endl;
    
    // Memory visualization
    std::cout << "Memory visualization:" << std::endl;
    std::cout << memoryAllocator->visualizeMemory() << std::endl;
}