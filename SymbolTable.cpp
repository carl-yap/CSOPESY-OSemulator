#include "SymbolTable.h"

// private constructor
SymbolTable::SymbolTable() {
	table = std::unordered_map<uintptr_t, Symbol>();
}

bool SymbolTable::insert(uintptr_t address, const std::string& name, uint16_t value) {
	if (table.size() >= 32) {
		return false;
	}
	if (table.find(address) != table.end()) {
		update(address, value);
		return false;
	}
	table[address] = Symbol(name, value);
	return true;
}

bool SymbolTable::update(uintptr_t address, uint16_t newValue) {
	auto it = table.find(address);
	if (it == table.end()) return false;
	it->second.value = newValue;
	return true;
}

bool SymbolTable::update(const std::string& name, uint16_t newValue) {
	for (auto& [addr, symbol] : table) {
		if (strcmp(symbol.name.c_str(), name.c_str()) == 0) {
			symbol.value = newValue;
			return true;
		}
	}
	return false; // Not found
}

uint16_t SymbolTable::get(uintptr_t address) {
	auto it = table.find(address);
	if (it == table.end()) return NULL;
	return it->second.value;
}

uint16_t SymbolTable::get(const std::string& name) {
	for (const auto& [addr, symbol] : table) {
		if (strcmp(symbol.name.c_str(), name.c_str()) == 0) {
			return symbol.value;
		}
	}
	return NULL; // Not found
}

uintptr_t SymbolTable::generateAddress() {
	static uintptr_t nextAddress = 0x0;
	return nextAddress++;
}

bool SymbolTable::isValidAddress(uintptr_t address) const {
	// TODO: Implement address validation logic if needed
	return true;
}

void SymbolTable::print() const {
	for (const auto& [addr, symbol] : table) {
		std::cout << addr << " - " << symbol.name << " = " << symbol.value << std::endl;
	}
}