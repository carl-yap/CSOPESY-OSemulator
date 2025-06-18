#pragma once

#include <iostream>
#include <sstream>
#include <string>

class Console {
public:
	typedef std::string String;

	Console(String s) : name(s) {}
	virtual ~Console() = default;

	// abstract methods
	virtual void onEnabled() = 0;
	virtual void display() const = 0;
	virtual void process() = 0;

	String getName() const { return this->name; }

	friend class ConsoleManager;

protected:
	String name;
	String command;
};

