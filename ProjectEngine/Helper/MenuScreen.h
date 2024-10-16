#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>

#include "Platform.h"

class MenuScreen {
public:
	MenuScreen();

	void addOption(std::string opt);

	void clearOptions();

	void setWriteBefore(std::string toWrite);

	void update(char lastKeyStroke);

	bool optionWasEntered();
	int getCurrentOption();

private:
	void reWrite();

	bool hasOptions = false;

	bool optionEntered = false;
	int optionPointer = 0;
	std::vector<std::string> opts;
	std::string writeBefore;
};