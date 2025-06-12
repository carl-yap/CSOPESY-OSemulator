#include "Commands.h"
#include "SymbolTable.h"

/* ===============COMMAND INTERFACE================== */
ICommand::ICommand(int pid, ICommand::CommandType cmdType) : pid(pid), cmdType(cmdType) {}

ICommand::CommandType ICommand::getCommandType() {
	return this->cmdType;
}

void ICommand::execute() {
	std::this_thread::sleep_for(std::chrono::milliseconds(10)); // arbitrary delay time
}

/* ===============PRINT COMMAND================== */
PrintCommand::PrintCommand(int pid, String toPrint) : ICommand(pid, PRINT) {
	this->toPrint = toPrint;
}

void PrintCommand::execute() {
	ICommand::execute();

	std::stringstream msg;
	msg << "PID: " << this->pid << ":" << this->toPrint << std::endl;
}

/* ===============VARIABLE DECLARATION COMMAND================== */
DeclareCommand::DeclareCommand(int pid, String& varName, uint16_t value) : ICommand(pid, DECLARE) {
	this->varName = varName;
	this->value = value;
}

void DeclareCommand::execute() {
	ICommand::execute();
	performDeclaration();

	std::stringstream msg;
	msg << "Executing DECLARE command for PID " << this->pid
		<< ": variable '" << this->varName << "' with value " << this->value << std::endl;
}

void DeclareCommand::performDeclaration() {
	// TODO: symbol table association + placement in memory
	// Lecture note: Consider std::move or move-based operation for data transfer w/o copying

	// idk if this is right...
	SymbolTable::getInstance().insert(varName, value);
}

/* ===============ADD COMMAND================== */
AddCommand::AddCommand(int pid, String& var1, String& var2) : ICommand(pid, ADD) {
	this->var1 = var1;
	this->var2 = var2;
	this->sum = 0;
}

void AddCommand::execute() {
	ICommand::execute();
	performAddition();

	std::stringstream msg;
	msg << "Executing ADD command for PID " << this->pid
		<< ": " << this->var1 << " + " << this->var2 << " = " << this->sum << std::endl;
}

void AddCommand::performAddition() {
	// symbol table retrieval + add values
	uint16_t a = SymbolTable::getInstance().get(var1);
	uint16_t b = SymbolTable::getInstance().get(var2);
	this->sum = a + b;
}

/* ===============SUBTRACT COMMAND================== */
SubtractCommand::SubtractCommand(int pid, String& var1, String& var2) : ICommand(pid, SUBTRACT) {
	this->var1 = var1;
	this->var2 = var2;
	this->diff = 0;
}

void SubtractCommand::execute() {
	ICommand::execute();
	performSubtraction();

	std::stringstream msg;
	msg << "Executing SUBTRACT command for PID " << this->pid
		<< ": " << this->var1 << " + " << this->var2 << " = " << this->diff << std::endl;
}

void SubtractCommand::performSubtraction() {
	// symbol table retrieval + subtract values
	uint16_t a = SymbolTable::getInstance().get(var1);
	uint16_t b = SymbolTable::getInstance().get(var2);
	this->diff = a - b;
}

/* ===============SLEEP COMMAND================== */
SleepCommand::SleepCommand(int pid, int duration) : ICommand(pid, SLEEP) {
	this->duration = duration;
}

void SleepCommand::execute() {
	ICommand::execute();
	std::this_thread::sleep_for(std::chrono::milliseconds(this->duration));

	std::stringstream msg;
	msg << "Executing SLEEP command for PID " << this->pid 
		<< ": slept for " << this->duration << " milliseconds" << std::endl;
}

/* ===============FOR COMMAND================== */
ForCommand::ForCommand(int pid, int start, int end, std::vector<ICommand*>& commands) : ICommand(pid, FOR) {
	this->start = start;
	this->end = end;
	this->commands = commands;
}

void ForCommand::execute() {
	ICommand::execute();
	performCommandLoop();
}

void ForCommand::performCommandLoop() {
	for (int i = start; i < end; i++) {
		for (auto* cmd : commands) {
			if (cmd) cmd->execute();
		}
	}
}