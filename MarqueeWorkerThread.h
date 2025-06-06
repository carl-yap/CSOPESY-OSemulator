#pragma once

#include <iostream>
#include <chrono>		// For milliseconds
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <algorithm>
#include <deque>

#include <windows.h>
#include <conio.h> 		// For _kbhit() and _getch()

class MarqueeWorkerThread {
private:
	inline static const int REFRESH_DELAY = 10; // screen refresh in marquee console
	inline static const int POLLING_DELAY = 5; // keyboard polling rate; lower = better

	inline static std::string marqueeText = "Hello World in Marquee!";
	inline static std::mutex marqueeMutex;

	inline static std::string commandInput = "";
	inline static std::mutex commandMutex;

	inline static std::deque<std::string> commandHistory;
	inline static std::mutex commHistMutex;

	inline static std::atomic_bool quitFlag = false;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	
public:
	MarqueeWorkerThread() = default;

	void setCursorPosition(short x, short y);
	void clearLine(short y, short width);
	int  getConsoleWidth();

	inline static const int N_THREADS = 2;
	void refreshThread();
	void kbpollThread();

	static bool isRunning() { return !quitFlag; }
	static void reset() { quitFlag.store(false); }
};