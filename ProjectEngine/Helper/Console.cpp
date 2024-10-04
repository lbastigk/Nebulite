#include "Console.h"


Console::Console() {

}

void Console::refresh() {
	if (Platform::hasKeyBoardInput()) {
		char c = Platform::getCharacter();
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

bool Console::hasInput() {
	return !!consoleBuffer.size();
}

bool Console::hasInputInTemp() {
	return !!consoleBufferTemp.size();
}

std::string Console::getInput() {
	std::string tmp = consoleBuffer;
	consoleBuffer.clear();
	return tmp;
}

char Console::getLastKeystroke() {
	return _lastKeystroke;
}

bool Console::isNewLastKeystroke() {
	if (_isNewLastKeystroke) {
		_isNewLastKeystroke = false;
		return true;
	}
	else {
		return false;
	}
}