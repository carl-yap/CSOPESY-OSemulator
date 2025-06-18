#include "ConsoleManager.h"
#include "ProcessScheduler.h"

int main() {
    ConsoleManager::getInstance().init();
    ConsoleManager::getInstance().showMainConsole();

	ProcessScheduler::getInstance().init();

    //int loopCount = 0;
    
    bool running = true;
    while (running) {
		std::shared_ptr<Console> console = ConsoleManager::getInstance().getCurrentConsole();

        //std::cout << "Loop iteration: " << loopCount << " at " << console->getName() << std::endl;

        console->process();
		if (!ConsoleManager::getInstance().isRunning()) { return 0; } // Exit if process indicates to stop

        console = ConsoleManager::getInstance().getCurrentConsole();  // refresh
        console->display();

        running = ConsoleManager::getInstance().isRunning();
        //loopCount++;
    }

    return 0;
}