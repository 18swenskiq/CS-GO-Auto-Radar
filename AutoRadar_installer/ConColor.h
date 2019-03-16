#pragma once
#include <Windows.h>
#include <stdint.h>

HANDLE hConsole;
uint16_t consoleColorDefault;

namespace cc {

	void setup() {
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(hConsole, &info);

		consoleColorDefault = info.wAttributes;
	}

	void reset() {
		SetConsoleTextAttribute(hConsole, consoleColorDefault);
	}

	void error() {
		SetConsoleTextAttribute(hConsole, 12);
	}

	void warning() {
		SetConsoleTextAttribute(hConsole, 14);
	}

	void info() {
		SetConsoleTextAttribute(hConsole, 8);
	}

	void success() {
		SetConsoleTextAttribute(hConsole, 10);
	}

	void fancy() {
		SetConsoleTextAttribute(hConsole, 13);
	}
}