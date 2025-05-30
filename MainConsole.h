#pragma once

#include "AConsole.h"

class MainConsole : public Console {
public:
	MainConsole() : Console("MAIN_CONSOLE") {}

	void onEnabled() override {}

	void display() const override {
		std::cout << "  ___  ____   __  ____  ____  ____  _  _ " << std::endl;
		std::cout << " / __)/ ___) /  \\(  _ \\(  __)/ ___)( \\/ )" << std::endl;
		std::cout << "( (__ \\___ \\(  O )) __/ ) _) \\___ \\ )  / " << std::endl;
		std::cout << " \\___)(____/ \\__/(__)  (____)(____/(__/  " << std::endl;
		std::cout << "Hello, Welcome to CSOPESPY commandline!" << std::endl;
		std::cout << "Type 'exit' to quit, 'clear' to clear the screen" << std::endl;
	}

	void process() override {
	}
};