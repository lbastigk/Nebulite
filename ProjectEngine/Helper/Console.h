#pragma once


#include <string>
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
	std::string consoleBuffer;
	std::string consoleBufferTemp;

	char _lastKeystroke = ' ';
	bool _isNewLastKeystroke = false;
};