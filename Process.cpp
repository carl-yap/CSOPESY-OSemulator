#include "Process.h"

Process::Process(int id, const std::string& name, int minIns, int maxIns) {
	this->pid = id;
	this->name = name;
	this->state = State::NEW;
	this->programCounter = 0;

	// Generate a random number of instructions of length [minIns, maxIns]
	generateInstructionsBetween(minIns, maxIns);

	// set the arrival time to NOW
	this->arrivalTime = std::chrono::system_clock::now();
}

void Process::generateInstructionsBetween(int min, int max) {
	int n = min + rand() % (max - min + 1);
	this->instructions.resize(n);
	for (int i = 0; i < n; ++i) {
		// TODO: implement random instruction generation logic

		this->instructions[i] = std::make_shared<SleepCommand>(this->pid, 100);
	}
}

/*============== GETTERS ================*/
Process::State Process::getState() const {
	return this->state;
}

int Process::getPID() const {
	return this->pid;
}

int Process::getCmdListSize() const {
	return static_cast<int>(this->instructions.size());
}

std::string Process::getName() const {
	return this->name;
}

std::string Process::getCounter() const {
	return std::to_string(this->programCounter) + "/" + std::to_string(this->instructions.size());
}

Process::TimePoint Process::getArrivalTime() const {
	return this->arrivalTime;
}

Process::TimePoint Process::getEndTime() const {
	return this->endTime;
}

Process::TimePoint Process::getStartTime() const {
	return this->startTime;
}

const std::vector<Process::LogEntry>& Process::getLogs() const {
    return logs;
}

/*============== SETTERS ================*/
void Process::setState(State newState) {
	this->state = newState;
}

void Process::setEndTime(TimePoint endTime) {
	this->endTime = endTime;
}

void Process::setStartTime(TimePoint startTime) {
	this->startTime = startTime;
}

/*============== SUBROUTINES ================*/
void Process::executeCurrentCommand() const {
	this->instructions[this->programCounter]->execute();
}

void Process::moveToNextLine() {
	this->programCounter++;
}

int Process::getRemainingBurstTime() const {
	int totalSize = static_cast<int>(this->instructions.size());
	return totalSize - this->programCounter;
}

bool Process::isFinished() const {
	return this->programCounter >= this->instructions.size();
}

void Process::addLog(int core, const std::string& message) {
    logs.push_back(LogEntry{ std::chrono::system_clock::now(), core, message });
}

void logHelloWorld(Process* proc, int coreID) {
    proc->addLog(coreID, "Hello world from " + proc->getName());
}

