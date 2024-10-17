#include "Console.h"


Console::Console() {

}

void Console::refresh() {
	if (Platform::hasKeyBoardInput()) {
		int c = Platform::getCharacter();
		_lastKeystroke = c;
		_isNewLastKeystroke = true;

		//Enter
		if (c == Platform::KeyPress::Enter) {
			consoleBuffer = consoleBufferTemp;
			consoleBufferTemp.clear();
		}
		//Backspace
		else if (c == Platform::KeyPress::Backspace) {
			Platform::putCharacter(Platform::KeyPress::Backspace);
			Platform::putCharacter(Platform::KeyPress::Space);
			Platform::putCharacter(Platform::KeyPress::Backspace);

			//Delete last from string
			if (!consoleBufferTemp.empty()) {
				consoleBufferTemp.pop_back();
			}
		}
		else {
			//Append to string
			consoleBufferTemp.push_back(c);

			//Add to out
			if(!Platform::hasDefaultEcho){
				Platform::putCharacter(c);
			}
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
	std::string tmp = Platform::vectorToString(consoleBuffer);
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