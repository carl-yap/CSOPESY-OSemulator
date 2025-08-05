#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

class SymbolTable {

struct Symbol {
	std::string name;
	uint16_t value;
};

private:
	std::unordered_map<uintptr_t, Symbol> table;

public:
	SymbolTable();

	bool insert(uintptr_t address, const std::string& name, uint16_t value);
	bool update(uintptr_t address, uint16_t value);
	bool update(const std::string& name, uint16_t value);	
	uint16_t get(uintptr_t address);
	uint16_t get(const std::string& name);
	
	uintptr_t generateAddress();
	bool isValidAddress(uintptr_t address) const;
	void print() const;
	void clear() { table.clear(); }
};