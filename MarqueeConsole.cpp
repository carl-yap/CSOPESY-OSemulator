#include "MarqueeConsole.h"
#include "MarqueeWorkerThread.h"
#include "ConsoleManager.h"

MarqueeConsole::MarqueeConsole() : Console("MARQUEE_CONSOLE") {}

void MarqueeConsole::onEnabled() { /* no op */ }

void MarqueeConsole::display() const { /* no op */ }

void MarqueeConsole::process() {
	// reset quit flag before starting threads
	MarqueeWorkerThread::reset();

	std::vector<MarqueeWorkerThread> workers;
	std::vector<std::thread> threads;

	for (int i = 0; i < MarqueeWorkerThread::N_THREADS; i++) {
		workers.push_back(MarqueeWorkerThread());
	}

	threads.push_back(std::thread(&MarqueeWorkerThread::refreshThread, &workers[0]));
	threads.push_back(std::thread(&MarqueeWorkerThread::kbpollThread, &workers[1]));

	// make cursor invisible
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	ConsoleManager::getInstance().clearScreen();

	while (MarqueeWorkerThread::isRunning()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	for (int i = 0; i < MarqueeWorkerThread::N_THREADS; i++) {
		if (threads[i].joinable()) {
			threads[i].join();
		}
	}

	// closing procedures
	cursorInfo.bVisible = TRUE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
	ConsoleManager::getInstance().showMainConsole();
	ConsoleManager::getInstance().clearScreen();
}