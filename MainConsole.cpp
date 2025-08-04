#include "MainConsole.h"
#include "ConsoleManager.h"
#include "ProcessScheduler.h"

MainConsole::MainConsole() : Console("MAIN_CONSOLE") {
    this->command = "";
}

void MainConsole::onEnabled() { /* no-op */ }

void MainConsole::display() const {
    std::cout << "  ___  ____   __  ____  ____  ____  _  _ " << std::endl;
    std::cout << " / __)/ ___) /  \\(  _ \\(  __)/ ___)( \\/ )" << std::endl;
    std::cout << "( (__ \\___ \\(  O )) __/ ) _) \\___ \\ )  / " << std::endl;
    std::cout << " \\___)(____/ \\__/(__)  (____)(____/(__/  " << std::endl;
    std::cout << "Hello, Welcome to CSOPESPY commandline!" << std::endl;
    std::cout << "Type 'exit' to quit, 'clear' to clear the screen" << std::endl;
}

void MainConsole::process() {
	while (ConsoleManager::getInstance().getCurrentConsole()->getName() == "MAIN_CONSOLE") {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);

        std::vector<std::string> commandTokens = tokenizeString(command);
        std::string lowerCommand = toLower(commandTokens[0]);

        if (lowerCommand == "exit") {
            std::cout << "exit command recognized. Exiting application." << std::endl;
            // Cleanup
            ConsoleManager::getInstance().setRunStatus(false);
            ProcessScheduler::getInstance().exit();
            return;
        }
        else if (lowerCommand == "clear") {
		    ConsoleManager::getInstance().clearScreen();
            display();
        }
        else if (lowerCommand == "initialize") {
            handleInitialize();
        }
        else if (lowerCommand == "screen") {
            handleScreen(commandTokens);
        }
        else if (lowerCommand == "scheduler-start") {
            handleSchedulerTest();
        }
        else if (lowerCommand == "scheduler-stop") {
            handleSchedulerStop();
        }
        else if (lowerCommand == "report-util") {
            handleReportUtil();
        }
        else if (lowerCommand == "marquee") {
            handleMarquee();
        }
        else if (lowerCommand == "process-smi") {
            ProcessScheduler::getInstance().showProcessList();
        }
        else {
            std::cout << "Unknown command. Available commands: initialize, marquee, screen, scheduler-test, scheduler-stop, report-util, clear, exit" << std::endl;
        }
    }
    ConsoleManager::getInstance().clearScreen();
}

Console::String MainConsole::toLower(const String& str) {
    String result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

std::vector<std::string> MainConsole::tokenizeString(const String& input) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    bool escapeNext = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (inQuotes) {
            if (c == '"' and !escapeNext) { // end quote mark
                inQuotes = false;
                tokens.push_back(token);
                token.clear();
			}
			else if (c == '\\' && !escapeNext) { // handle escape character
                escapeNext = true;
            } 
            else {
                token += c;
                if (escapeNext) { escapeNext = false; }
            }
        } else {
            if (std::isspace(c)) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
			}
			else if (c == '"') { // start quote mark
                inQuotes = true;
            } 
            else 
            {
                token += c;
            }
        }
    }

	if (!token.empty()) { tokens.push_back(token); } // Residual characters after last space
    return tokens;
}

std::vector<std::vector<std::string>> MainConsole::tokenizeCustomCommands(const String& commands) {
    std::vector<std::vector<std::string>> result;
    std::istringstream commandStream(commands);
    String segment;

    // Split by ';'
    while (std::getline(commandStream, segment, ';')) {
        std::vector<std::string> tokens;
        String token;
        std::istringstream tokenStream(segment);

        // Tokenize by spaces and parentheses
        char c;
        while (tokenStream.get(c)) {
            if (std::isspace(c) || c == '(' || c == ')') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                if (c == '(' || c == ')') {
                    tokens.push_back(String(1, c));
                }
            } else {
                token += c;
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }
        // Remove empty tokens
        tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
            [](const std::string& s) { return s.empty(); }), tokens.end());

        if (!tokens.empty()) {
            result.push_back(tokens);
        }
    }
    return result;
}

void MainConsole::handleInitialize() {
    std::cout << "initialize command recognized. Doing something." << std::endl;
}

void MainConsole::handleScreen(std::vector<std::string> commandTokens) {
    String name = "empty";
    bool resume = false;

    switch (commandTokens.size()) {
    case 1:
        std::cout << "screen command recognized. No additional parameters provided." << std::endl;
		break;
    case 2:
        if (commandTokens[1] != "-ls") {
            std::cout << "Process name is required. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
            break;
		}
        ProcessScheduler::getInstance().showScreenList();
        return;
    case 3:
        if (commandTokens[1] != "-r" && commandTokens[1] != "-s") {
            std::cout << "Syntax for screen command is incorrect. Try using screen -r <process_name> or screen -s <process_name>." << std::endl;
            break;
        }
        resume = (commandTokens[1] == "-r");
		name = commandTokens[2];
        ConsoleManager::getInstance().openScreen(name);
        return;
    case 4:
        if (commandTokens[1] == "-c") {
            name = commandTokens[2];
            std::vector<std::vector<String>> commands = tokenizeCustomCommands(commandTokens[3]);
            for (const auto& cmd : commands) {
                if (cmd.empty()) continue; // Skip empty commands
                std::string cmdStr = cmd[0];
                for (size_t i = 1; i < cmd.size(); ++i) {
                    cmdStr += " " + cmd[i];
                }
                // std::cout << "Found command: " << cmdStr << std::endl;
            }
            ConsoleManager::getInstance().customScreen(name, commands);
        }
        else if (commandTokens[1] == "-s") {
            size_t requiredMem = 0;  
            try {  
                requiredMem = std::stoull(commandTokens[3]);  
            } catch (const std::exception& e) {  
                std::cerr << "Exception caught: " << e.what() << std::endl;  
                return;  
            } 
            if (!ProcessScheduler::getInstance().isValidMemorySize(requiredMem)) {
				std::cerr << "Memory size out of range. Please enter a smaller number." << std::endl;
                return;
            }
            else {
				ConsoleManager::getInstance().startScreen(name, requiredMem);
                return;
            }
        }
        return;
    default:
        std::cout << "Too many parameters provided. The options for 'screen' are <-r|-s|-c>." << std::endl;
		return;
    }
}

void MainConsole::handleSchedulerTest() {
    ProcessScheduler::getInstance().start();
}

void MainConsole::handleSchedulerStop() {
    std::cout << "scheduler-stop command recognized. Shutting down scheduler." << std::endl;
    ProcessScheduler::getInstance().stop();
}

void MainConsole::handleReportUtil() {
    ProcessScheduler::getInstance().makeReportUtil();
}

void MainConsole::handleMarquee() {
    ConsoleManager::getInstance().openMarquee();
}