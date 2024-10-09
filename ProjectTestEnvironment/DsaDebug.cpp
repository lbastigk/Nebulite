#include "DsaDebug.h"

double DsaDebug::getMemoryUsagekB() {
    return Platform::getMemoryUsagekB();
}

void DsaDebug::printWithPrefix(const std::string& str, const std::string& prefix) {
    std::istringstream iss(str);
    std::string line;

    while (std::getline(iss, line)) {
        std::cout << prefix << line << '\n';
    }
}

int DsaDebug::menueScreen(const std::string options, int startoption,std::string before, std::string after, bool showAll) {
    //Get line count
    std::stringstream ss(options);
    std::string line;
    int i = 0;
    while (std::getline(ss, line)) {
        i++;
    }
    int linecount = i;
    int option = startoption;
    
    int input = ' ';
    while (input != Platform::KeyPress::Enter) {
        if (input == 'w' || input == Platform::KeyPress::arrowUp) {
            option -= 1;
            option += linecount;
            option %= linecount;
        }
        else if (input == 's' || input == Platform::KeyPress::arrowDown) {
            option += 1;
            option %= linecount;
        }
        Platform::clearScreen();
        std::cout << before;
        ss.clear();
        ss.str(options);
        

        // print lines
        int lineDiff = 10;
        int i = 0;
        while (std::getline(ss, line)) {
            if (!showAll && i - option == -lineDiff) {
                std::cout << "    ...\n";
            }
            if (!showAll && i - option == lineDiff) {
                std::cout << "    ...\n";
            }

            if (showAll || (i - option > -lineDiff && i - option < lineDiff)) {
                if (i == option) {
                    std::cout << "  ->\t" << line << "\n";
                }
                else {
                    std::cout << "    \t" << line << "\n";
                } 
            } 
            i++;
        }
        std::cout << after;

        //Get new input
        while (!Platform::hasKeyBoardInput()) {
            //wait for input
        }
        input = Platform::getCharacter();
    }
    return option;
}

void DsaDebug::menueScreenTemplate() {
    Platform::clearScreen();
    int opt = 0;
    int ranges = -1;
    std::string options = "Example1\nQuit";
    while (opt != ranges) {
        opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");
        Platform::clearScreen();

        switch (opt) {
        case 0:

            break;
        }
        //ranges = std::ranges::count(options, '\n');
    }

}


