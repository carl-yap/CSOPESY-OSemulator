#pragma once

#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>

#include "SymbolTable.h"

// command interface
class ICommand {
public:
	typedef std::string String;
	typedef std::shared_ptr<SymbolTable> TablePtr;
	enum CommandType {
		PRINT,
		DECLARE,
		ADD,
		SUBTRACT,
		READ,
		WRITE,
		SLEEP,
		FOR
	};

	ICommand(int pid, CommandType cmdType);
	ICommand(int pid, CommandType cmdType, TablePtr table);
	virtual ~ICommand() = default;

	CommandType getCommandType();
	virtual String getOutput() const = 0;
	virtual void execute();

protected:
	int pid;
	CommandType cmdType;
	TablePtr table; // Optional, can be nullptr if not needed
	
};

class PrintCommand : public ICommand {
public:
	PrintCommand(int pid, TablePtr table, std::vector<String> printTokens);
	String getOutput() const override { return output; }
	void execute() override;
	void parsePrintTokens();

private:
	std::vector<String> printTokens;
	String toPrint;
	String output = "empty";
};

class DeclareCommand : public ICommand {
public:
	DeclareCommand(int pid, TablePtr table, String varName, uint16_t value);
	String getOutput() const override { return output; }
	void execute() override;
	void performDeclaration();

private:
	String varName;
	uint16_t value;
	String output = "empty";
};

class AddCommand : public ICommand {
public:
	AddCommand(int pid, TablePtr table, String out, String var1, String var2);
	String getOutput() const override { return output; }
	void execute() override;
	void performAddition();

private:
	String out;
	String var1;
	String var2;
	uint16_t sum; 
	String output = "empty";
};

class SubtractCommand : public ICommand {
public:
	SubtractCommand(int pid, TablePtr table, String out, String var1, String var2);
	String getOutput() const override { return output; }
	void execute() override;
	void performSubtraction();

private:
	String out;
	String var1;
	String var2;
	uint16_t diff;
	String output = "empty";
};

class ReadCommand : public ICommand {
public:
	ReadCommand(int pid, TablePtr table, String outVar, uintptr_t address);
	String getOutput() const override { return output; }
	void execute() override;
	void performRead();
private:
	String outVar;
	uintptr_t address;
	String output = "empty"; 
};

class WriteCommand : public ICommand {
public:
	WriteCommand(int pid, TablePtr table, uintptr_t address, uint16_t value);
	WriteCommand(int pid, TablePtr table, uintptr_t address, String varName);
	String getOutput() const override { return output; }
	void execute() override;
	void performWrite();
private:
	uintptr_t address;
	uint16_t value;
	String varName; // Optional, if writing to a variable
	String output = "empty";
};

class SleepCommand : public ICommand {
public:
	SleepCommand(int pid, int duration);
	String getOutput() const override;
	void execute() override;

private:
	int duration;
};

class ForCommand : public ICommand {
public:
	ForCommand(int pid, int start, int end, std::vector<ICommand*>& commands);
	void execute() override;
	void performCommandLoop();

private:
	int start;
	int end;
	std::vector<ICommand*> commands;
};