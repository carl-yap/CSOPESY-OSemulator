#include "SymbolTable.h"

// Singleton instance
SymbolTable& SymbolTable::getInstance() {
	static SymbolTable instance;
	return instance;
}

// private constructor
SymbolTable::SymbolTable() {}

// create empty table
void SymbolTable::init() {
	this->table = std::unordered_map<std::string, uint16_t>();
}

bool SymbolTable::insert(const std::string& name, uint16_t value) {
	if (table.find(name) != table.end()) return false;
	table[name] = value;
	return true;
}

bool SymbolTable::update(const std::string& name, uint16_t value) {
	auto it = table.find(name);
	if (it == table.end()) return false;
	it->second = value;
	return true;
}

uint16_t SymbolTable::get(const std::string& name) {
	auto it = table.find(name);
	if (it == table.end()) return NULL;
	return it->second;
}

void SymbolTable::print() const {
	for (const auto& [name, value] : table) {
		std::cout << name << " = " << value << std::endl;
	}
}