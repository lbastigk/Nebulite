#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>

class MenueScreen {
public:
	MenueScreen() {
		opts.push_back("No options created!");
	}

	void addOption(std::string opt) {
		if (!hasOptions) {
			opts.clear();
			hasOptions = true;
		}
		opts.push_back(opt);
	}

	void clearOptions() {
		opts.clear();
		opts.push_back("No options created!");
		hasOptions = false;
	}

	void setWriteBefore(std::string toWrite) {
		writeBefore = toWrite;
	}

	void update(char lastKeyStroke) {
		if (lastKeyStroke == 'w') {

		}
		else if (lastKeyStroke == 's') {
			optionPointer = (optionPointer + opts.size() + 1) % opts.size();
		}
		else if (lastKeyStroke == '\r') {
			optionEntered = true;
		}

	}

	bool optionWasEntered() {
		return optionEntered;
	}
	int getCurrentOption() {
		return optionPointer;
	}

private:
	void reWrite() {
		system("cls");
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

	bool hasOptions = false;

	bool optionEntered = false;
	int optionPointer = 0;
	std::vector<std::string> opts;
	std::string writeBefore;
};