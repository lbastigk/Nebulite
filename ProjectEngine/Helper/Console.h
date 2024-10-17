#pragma once


#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>


#include "Platform.h"


class Console {
public:
	Console();
	void refresh();
	bool hasInput();
	bool hasInputInTemp();
	std::string getInput();
	char getLastKeystroke();
	bool isNewLastKeystroke();
private:
	std::vector<int> consoleBuffer;
	std::vector<int> consoleBufferTemp;

	int _lastKeystroke = Platform::KeyPress::Space;
	bool _isNewLastKeystroke = false;
};