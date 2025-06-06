#pragma once

#include "AConsole.h"

class MarqueeConsole : public Console {
public:
	MarqueeConsole();

	void onEnabled() override;
	void display() const override;
	void process() override;
};