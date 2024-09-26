#pragma once

#include <string>
#include <stdio.h>
#include <iostream>
#include <conio.h>

class Console {
public:
	Console() {

	}

	void refresh() {
		if (kbhit()) {
			char c = _getch();
			_lastKeystroke = c;
			_isNewLastKeystroke = true;

			//Enter
			if (c == '\r') {
				consoleBuffer = consoleBufferTemp;
				consoleBufferTemp.clear();
			}
			//Backspace
			else if (c == 0x08) {
				std::cout << c;
				std::cout << ' ';
				std::cout << c;

				//Delete last from string
				if (!consoleBufferTemp.empty()) {
					consoleBufferTemp.pop_back();
				}
			}
			else {
				//Append to string
				consoleBufferTemp += c;

				//Add to out
				std::cout << c;
			}
		}
	}

	bool hasInput() {
		return !!consoleBuffer.size();
	}

	bool hasInputInTemp() {
		return !!consoleBufferTemp.size();
	}

	std::string getInput() {
		std::string tmp = consoleBuffer;
		consoleBuffer.clear();
		return tmp;
	}

	char getLastKeystroke() {
		return _lastKeystroke;
	}
	bool isNewLastKeystroke() {
		if (_isNewLastKeystroke) {
			_isNewLastKeystroke = false;
			return true;
		}
		else {
			return false;
		}
		return isNewLastKeystroke;
	}

private:
	std::string consoleBuffer;
	std::string consoleBufferTemp;

	char _lastKeystroke = ' ';
	bool _isNewLastKeystroke = false;
};