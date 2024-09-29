#include "MenueScreen.h"

MenueScreen::MenueScreen() {
	opts.push_back("No options created!");
}

void MenueScreen::addOption(std::string opt) {
	if (!hasOptions) {
		opts.clear();
		hasOptions = true;
	}
	opts.push_back(opt);
}

void MenueScreen::clearOptions() {
	opts.clear();
	opts.push_back("No options created!");
	hasOptions = false;
}

void MenueScreen::setWriteBefore(std::string toWrite) {
	writeBefore = toWrite;
}

void MenueScreen::update(char lastKeyStroke) {
	if (lastKeyStroke == 'w') {

	}
	else if (lastKeyStroke == 's') {
		optionPointer = (optionPointer + opts.size() + 1) % opts.size();
	}
	else if (lastKeyStroke == '\r') {
		optionEntered = true;
	}

}

bool MenueScreen::optionWasEntered() {
	return optionEntered;
}
int MenueScreen::getCurrentOption() {
	return optionPointer;
}

void MenueScreen::reWrite() {
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
