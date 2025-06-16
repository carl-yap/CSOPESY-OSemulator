#pragma once

#include <sstream>
#include <vector>
#include <iterator>
#include "AConsole.h"
#include "FCFSScheduler.h"

class MainConsole : public Console {
public:
    MainConsole();

    void onEnabled() override;
    void display() const override;
    void process() override;

private:
	String  command;
    String  toLower(const String& str);
    std::vector<String> tokenizeString(const String& input);

    std::shared_ptr<FCFSScheduler> scheduler;
    
    void handleInitialize();
    void handleScreen(std::vector<String> commandTokens);
    void handleSchedulerTest();
    void handleSchedulerStop();
    void handleReportUtil();
    void handleMarquee();
};