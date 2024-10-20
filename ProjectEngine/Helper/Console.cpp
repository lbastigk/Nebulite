#include "Console.h"


Console::Console() {

}

//returns true if new input
bool Console::refresh(bool enterMeansReturnBuffer) {
	int c = Platform::getCharacter();
	if (c) {
		_lastKeystroke = c;
		_isNewLastKeystroke = true;

		//Enter
		if (c == Platform::KeyPress::Enter && enterMeansReturnBuffer) {
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
	return !!c;
}

bool Console::hasInput() {
	return !!consoleBuffer.size();
}

int Console::inputSize(){
	return consoleBuffer.size();
}

bool Console::hasInputInTemp() {
	return !!consoleBufferTemp.size();
}

int Console::tempInputSize() {
	return consoleBufferTemp.size();
}



std::string Console::getInput(bool clearBuffer) {
	std::string tmp = Platform::vectorToString(consoleBuffer);
	if(clearBuffer)consoleBuffer.clear();
	return tmp;
}

std::string Console::getTempInput(bool clearBuffer) {
	std::string tmp = Platform::vectorToString(consoleBufferTemp);
	if(clearBuffer)consoleBufferTemp.clear();
	return tmp;
}



int Console::getLastKeystroke() {
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