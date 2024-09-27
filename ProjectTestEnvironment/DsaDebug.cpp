#include <iostream>
#include <sstream>
#include <string>

//#include <Windows.h>
//#include <Psapi.h>
//#include <conio.h>

#include <ranges>
#include <algorithm>
#include <ctype.h>

#include "Platform.h"

#pragma once

class DsaDebug {
public:
    // Function to get used memory by the program itself on Windows, in kB
    static double getMemoryUsagekB() {
        return Platform::getMemoryUsagekB();
    }

    static void printWithPrefix(const std::string& str, const std::string& prefix) {
        std::istringstream iss(str);
        std::string line;

        while (std::getline(iss, line)) {
            std::cout << prefix << line << '\n';
        }
    }

    static int menueScreen(const std::string options, int startoption,std::string before = "", std::string after = "", bool showAll = true) {
        //Get line count
        std::stringstream ss(options);
        std::string line;
        int i = 0;
        while (std::getline(ss, line)) {
            i++;
        }
        int linecount = i;
        int option = startoption;
        char input = ' ';
        char input2 = ' ';
        while (input != '\r') {
            if (input == 'w' || input == -32 && input2 == 72) {
                option -= 1;
                option += linecount;
                option %= linecount;
            }
            else if (input == 's' || input == -32 && input2 == 80) {
                option += 1;
                option %= linecount;
            }
            system("cls");
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
            if (Platform::hasKeyBoardInput()) {
                input2 = Platform::getCharacter();
            }
            else {
                input2 = ' ';
            }
        }
        return option;
    }

private:
    void menueScreenTemplate() {
        system("cls");
        int opt = 0;
        int ranges = -1;
        std::string options = "Example1\nQuit";
        while (opt != ranges) {
            opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");
            system("cls");

            switch (opt) {
            case 0:

                break;
            }
            //ranges = std::ranges::count(options, '\n');
        }

    }
};


