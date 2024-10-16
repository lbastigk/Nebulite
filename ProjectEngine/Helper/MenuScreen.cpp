#include "MenuScreen.h"

MenuScreen::MenuScreen() {
	opts.push_back("No options created!");
}

void MenuScreen::addOption(std::string opt) {
	if (!hasOptions) {
		opts.clear();
		hasOptions = true;
	}
	opts.push_back(opt);
}

void MenuScreen::clearOptions() {
	opts.clear();
	opts.push_back("No options created!");
	hasOptions = false;
}

void MenuScreen::setWriteBefore(std::string toWrite) {
	writeBefore = toWrite;
}

void MenuScreen::update(char lastKeyStroke) {
	if (lastKeyStroke == 'w') {

	}
	else if (lastKeyStroke == 's') {
		optionPointer = (optionPointer + opts.size() + 1) % opts.size();
	}
	else if (lastKeyStroke == '\r') {
		optionEntered = true;
	}

}

bool MenuScreen::optionWasEntered() {
	return optionEntered;
}
int MenuScreen::getCurrentOption() {
	return optionPointer;
}

void MenuScreen::reWrite() {
	Platform::clearScreen();
	std::cout << writeBefore;

	int i = 0;
	for (auto opt : opts) {
		std::cout << " ";
		if (i == optionPointer) {
			std::cout << "->";
		}
		std::cout << "\t" << opt << "\n";
		i++;
	}
}
