#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

class SymbolTable {
private:
	SymbolTable();
	std::unordered_map<std::string, uint16_t> table;

public:
	static SymbolTable& getInstance();
	void init(); // create empty table

	bool insert(const std::string& name, uint16_t value);
	bool update(const std::string& name, uint16_t value);
	uint16_t get(const std::string& name);
	void print() const;
};