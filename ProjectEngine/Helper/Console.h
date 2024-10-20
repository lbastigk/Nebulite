#pragma once


#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>


#include "Platform.h"


class Console {
public:
	Console();
	bool refresh(bool enterMeansReturnBuffer = true);

	bool hasInput();
	bool hasInputInTemp();

	int inputSize();
	int tempInputSize();

	std::string getInput(bool clearBuffer = true);
	std::string getTempInput(bool clearBuffer = true);
	int getLastKeystroke();
	bool isNewLastKeystroke();
private:
	std::vector<int> consoleBuffer;
	std::vector<int> consoleBufferTemp;

	int _lastKeystroke = Platform::KeyPress::Space;
	bool _isNewLastKeystroke = false;
};