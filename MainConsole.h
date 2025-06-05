#pragma once

#include "AConsole.h"

class MainConsole : public Console {
public:
    MainConsole();

    void onEnabled() override;
    void display() const override;
    void process() override;

private:
	String  command;
    String  toLower(const String& str);
    void    tokenizeString(const String& input, String tokenArray[], int maxTokens=10);
    
    void handleInitialize();
    void handleScreen(const String commandTokens[]);
    void handleSchedulerTest();
    void handleSchedulerStop();
    void handleReportUtil();
    void handleMarquee();
};