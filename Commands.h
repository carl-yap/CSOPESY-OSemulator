#pragma once

#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>

// command interface
class ICommand {
public:
	typedef std::string String;
	enum CommandType {
		PRINT,
		DECLARE,
		ADD,
		SUBTRACT,
		SLEEP,
		FOR
	};

	ICommand(int pid, CommandType cmdType);
	CommandType getCommandType();
	virtual void execute();

protected:
	int pid;
	CommandType cmdType;
	
};

class PrintCommand : public ICommand {
public:
	PrintCommand(int pid, String toPrint);
	void execute() override;

private:
	String toPrint;
};

class DeclareCommand : public ICommand {
public:
	DeclareCommand(int pid, String& varName, uint16_t value);
	void execute() override;
	void performDeclaration();

private:
	String varName;
	uint16_t value;
};

class AddCommand : public ICommand {
	AddCommand(int pid, String& var1, String& var2);
	void execute() override;
	void performAddition();

private:
	String var1;
	String var2;
	uint16_t sum;
};

class SubtractCommand : public ICommand {
	SubtractCommand(int pid, String& var1, String& var2);
	void execute() override;
	void performSubtraction();

private:
	String var1;
	String var2;
	uint16_t diff;
};

class SleepCommand : public ICommand {
public:
	SleepCommand(int pid, int duration);
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