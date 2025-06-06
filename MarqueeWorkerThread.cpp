#include "MarqueeWorkerThread.h"

/* ======== Console Utility ======== */

// move the console cursor
void MarqueeWorkerThread::setCursorPosition(short x, short y) {
	COORD coord = { x, y };
	SetConsoleCursorPosition(this->hConsole, coord);
}

// clear a specific line (by overwriting with spaces)
void MarqueeWorkerThread::clearLine(short y, short width) {
	setCursorPosition(0, y);
	for (int i = 0; i < width; ++i) {
		std::cout << ' ';
	}
	setCursorPosition(0, y);
}

int MarqueeWorkerThread::getConsoleWidth() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(this->hConsole, &csbi);
	return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

/* ======== Thread Functions ======= */

void MarqueeWorkerThread::refreshThread() {
	int mXPos = 0; 
	int mXDir = 1; // 1 for right, -1 for left
	int mYPos = 3;
	int mYDir = 1; // 1 for down, -1 for up

	const int HEADER_ROW = 0;
	const int HEADER_ROW_END = 3;
	const int COMMAND_INPUT_ROW = 15;
	const int COMMAND_HISTORY_START = 16;
	const int MAX_COMMANDS_TO_DISPLAY = 5;

	while (!this->quitFlag) {
		int consoleWidth = getConsoleWidth();
		if (consoleWidth < 80) consoleWidth = 80; // assert minimum width

		int consoleHeight = COMMAND_HISTORY_START + MAX_COMMANDS_TO_DISPLAY + 2;

		// display header
		setCursorPosition(0, HEADER_ROW);
		std::cout << "*******************************************" << std::endl;
		setCursorPosition(0, HEADER_ROW + 1);
		std::cout << "* Displaying a marquee console! *" << std::endl;
		setCursorPosition(0, HEADER_ROW + 2);
		std::cout << "*******************************************" << std::endl;

		// clear marquee area
		for (int y = HEADER_ROW_END; y < COMMAND_INPUT_ROW; ++y) {
			clearLine(y, consoleWidth);
		}

		// get marquee content
		std::string currentMarqueeContent;
		{
			std::lock_guard<std::mutex> lock(this->marqueeMutex);
			currentMarqueeContent = this->marqueeText;
		}

		// calculate for position (X-axis)
		int textLength = currentMarqueeContent.length();

		if (textLength > consoleWidth) {
			mXPos = (consoleWidth - textLength) / 2; // center text
			mXDir = 0; // no scrolling needed
		}
		else {
			mXPos += mXDir; // move

			// reverse at boundary
			if (mXPos + textLength >= consoleWidth && mXDir == 1) {
				mXDir = -1; // go left
				mXPos = consoleWidth - textLength; // snap to edge
			}
			else if (mXPos <= 0 && mXDir == -1) {
				mXDir = 1; // go right
				mXPos = 0; // snap to edge
			}

			mXPos = (std::max)(0, (std::min)(mXPos, consoleWidth - textLength)); // assert staying in bounds
		}

		// calculate for position (Y-axis)
		const int MIN_Y = HEADER_ROW_END + 1;
		const int MAX_Y = COMMAND_INPUT_ROW - 1;
		if (MAX_Y > MIN_Y) {
			mYPos += mYDir;

			// reverse at boundaries
			if (mYPos >= MAX_Y && mYDir == 1) {
				mYDir = -1; // go up
				mYPos = MAX_Y;
			}
			else if (mYPos <= MIN_Y && mYDir == -1) {
				mYDir = 1; // go down
				mYPos = MIN_Y;
			}
		}
		else {
			mYPos = MIN_Y;
			mYDir = 0; // no vertical bounce
		}

		setCursorPosition(mXPos, mYPos);
		std::cout << currentMarqueeContent;

		// display user input
		setCursorPosition(0, COMMAND_INPUT_ROW);
		clearLine(COMMAND_INPUT_ROW, consoleWidth);
		std::cout << "Enter a Command: ";
		{
			std::lock_guard<std::mutex> lock(this->commandMutex);
			std::cout << this->commandInput;
		}

		// display command history
		setCursorPosition(0, COMMAND_HISTORY_START);
		{
			std::lock_guard<std::mutex> lock(this->commHistMutex);
			int curr = COMMAND_HISTORY_START;
			for (const auto& cmd : this->commandHistory) {
				if (curr < consoleHeight) {
					setCursorPosition(0, curr);
					clearLine(curr, consoleWidth);
					std::cout << "Command entered: " << cmd.substr(0, consoleWidth - 2) << std::endl;
					curr++;
				}
				else {
					break;
				}
			}
			// clean any extra lines
			for (int y = curr; y < consoleHeight - 1; ++y) {
				clearLine(y, consoleWidth);
			}
		}


		// sleep
		std::this_thread::sleep_for(std::chrono::milliseconds(this->REFRESH_DELAY));
	}
}

void MarqueeWorkerThread::kbpollThread() {
	while (!this->quitFlag) {
		if (_kbhit()) {
			char ch = _getch();

			if (ch == '\r') { // 'Enter' pressed
				std::string command;
				{
					std::lock_guard<std::mutex> lock(this->commandMutex);
					command = this->commandInput;
					this->commandInput.clear();
				}
				
				if (!command.empty()) {
					if (command == "exit") {
						this->quitFlag.store(true);
						break;
					}
					{
						std::lock_guard<std::mutex> lock(this->commHistMutex);
						this->commandHistory.push_front(command);
						const size_t MAX_HISTORY = 10;
						if (this->commandHistory.size() > MAX_HISTORY) {
							this->commandHistory.pop_back(); // remove oldest command over the limit
						}
					}
				}
			}
			else if (ch == '\b') { // 'Backspace' pressed
				std::lock_guard<std::mutex> lock(this->commandMutex);
				if (!this->commandInput.empty()) {
					this->commandInput.pop_back();
				}
			}
			else if (isprint(ch)) { // only add printable characters
				std::lock_guard<std::mutex> lock(this->commandMutex);
				this->commandInput += ch;
			}
		}
		//sleep
		std::this_thread::sleep_for(std::chrono::milliseconds(this->POLLING_DELAY));
	}
}