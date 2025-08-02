#include "Commands.h"

/* ===============COMMAND INTERFACE================== */
ICommand::ICommand(int pid, ICommand::CommandType cmdType) : pid(pid), cmdType(cmdType), table(nullptr) {}
ICommand::ICommand(int pid, ICommand::CommandType cmdType, TablePtr table) : pid(pid), cmdType(cmdType), table(table) {}

ICommand::CommandType ICommand::getCommandType() {
	return this->cmdType;
}

void ICommand::execute() {
	//if (table) { table->print(); }
	std::this_thread::sleep_for(std::chrono::milliseconds(100)); // arbitrary delay time
}

/* ===============PRINT COMMAND================== */
PrintCommand::PrintCommand(int pid, TablePtr table, std::vector<String> printTokens) : ICommand(pid, PRINT, table) {
	this->printTokens = printTokens;
	this->toPrint = "";
}

void PrintCommand::execute() {
	ICommand::execute();
	parsePrintTokens();

	std::stringstream msg;
	msg << this->toPrint;

	output = msg.str();
}

void PrintCommand::parsePrintTokens() {
	bool inQuotes = false;

	for (size_t i = 2; i < printTokens.size() - 1; ++i) {
		if (i > 2) toPrint += " "; // Add space between tokens
		if (inQuotes) {
			if (printTokens[i] == "\"") {
				inQuotes = false; // End of quoted string
			}
			else {
				toPrint += printTokens[i];
			}
		}
		else {
			if (printTokens[i].front() == '"') {
				inQuotes = true; // Start of quoted string
				toPrint += printTokens[i].substr(1);
			}
			else if (printTokens[i] == "+") {
				continue;
			}
			else { // case of a variable
				toPrint += std::to_string(table->get(printTokens[i]));
			}
		}
	}
}

/* ===============VARIABLE DECLARATION COMMAND================== */
DeclareCommand::DeclareCommand(int pid, TablePtr table, String varName, uint16_t value) : ICommand(pid, DECLARE, table) {
	this->varName = varName;
	this->value = value;
}

void DeclareCommand::execute() {
	ICommand::execute();
	performDeclaration();

	std::stringstream msg;
	msg << "Executing DECLARE command for PID " << this->pid
		<< ": variable '" << this->varName << "' with value " << this->value << std::endl;
	output = msg.str();
}

void DeclareCommand::performDeclaration() {
	// TODO: symbol table association + placement in memory
	// Lecture note: Consider std::move or move-based operation for data transfer w/o copying

	uintptr_t address = table->generateAddress();
	table->insert(address, varName, value);
}

/* ===============ADD COMMAND================== */
AddCommand::AddCommand(int pid, TablePtr table, String out, String var1, String var2) : ICommand(pid, ADD, table) {
	this->out = out;
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
	output = msg.str();
}

void AddCommand::performAddition() {
	// symbol table retrieval + add values
	uint16_t a = table->get(var1);
	uint16_t b = table->get(var2);
	this->sum = a + b;
	if (!table->update(out, sum)) { // See if the variable already exists
		uintptr_t address = table->generateAddress();
		table->insert(address, out, sum);
	}
}

/* ===============SUBTRACT COMMAND================== */
SubtractCommand::SubtractCommand(int pid, TablePtr table, String out, String var1, String var2) : ICommand(pid, SUBTRACT, table) {
	this->out = out;
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
	output = msg.str();
}

void SubtractCommand::performSubtraction() {
	// symbol table retrieval + subtract values
	uint16_t a = table->get(var1);
	uint16_t b = table->get(var2);
	this->diff = a - b;
	if (!table->update(out, diff)) { // See if the variable already exists
		uintptr_t address = table->generateAddress();
		table->insert(address, out, diff);
	}
}

/* ===============READ COMMAND================== */
ReadCommand::ReadCommand(int pid, TablePtr table, String outVar, uintptr_t address) : ICommand(pid, READ, table) {
	this->outVar = outVar;
	this->address = address;
}

void ReadCommand::execute() {
	ICommand::execute();
	performRead();

	std::stringstream msg;
	msg << "Executing READ command for PID " << this->pid
		<< ": read value from address " << std::hex << this->address 
		<< " into variable '" << this->outVar << "'" << std::endl;
	output = msg.str();
}

void ReadCommand::performRead() {
	uint16_t value = table->get(address);
	if (value != NULL) {
		if (!table->update(outVar, value)) { // See if the variable already exists
			uintptr_t address = table->generateAddress();
			table->insert(address, outVar, value);
		}
	} else { // If memory block not initialized, set to 0
		uintptr_t address = table->generateAddress();
		table->insert(address, outVar, 0);
	}
}

/* ===============WRITE COMMAND================== */
WriteCommand::WriteCommand(int pid, TablePtr table, uintptr_t address, uint16_t value) : ICommand(pid, WRITE, table) {
	this->address = address;
	this->value = value;
}

WriteCommand::WriteCommand(int pid, TablePtr table, uintptr_t address, String varName) : ICommand(pid, WRITE, table) {
	this->address = address;
	this->varName = varName;
	this->value = -1;
}

void WriteCommand::execute() {
	ICommand::execute();
	performWrite();

	std::stringstream msg;
	msg << "Executing WRITE command for PID " << this->pid
		<< ": writing value " << this->value << " to address " << std::hex << this->address << std::endl;
	output = msg.str();
}

void WriteCommand::performWrite() {  
	value = table->get(varName);
    if (table->get(address) != NULL) {
		table->update(address, value);
	} else {
		table->insert(address, "no_name", value);
	}
}

/* ===============SLEEP COMMAND================== */
SleepCommand::SleepCommand(int pid, int duration) : ICommand(pid, SLEEP) {
	this->duration = duration;
}

std::string SleepCommand::getOutput() const {
	return "ZZzz...";
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