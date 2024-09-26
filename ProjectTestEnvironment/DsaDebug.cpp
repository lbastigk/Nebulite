#include <iostream>
#include <sstream>
#include <string>

#include <Windows.h>
#include <Psapi.h>
#include <conio.h>

#include <algorithm>
#include <ctype.h>

#pragma once

class DsaDebug {
public:
    // Function to get used memory by the program itself on Windows, in kB
    static double getMemoryUsagekB() {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

        // Convert bytes to kilobytes as double
        double usedMemKB = static_cast<double>(pmc.PrivateUsage) / 1024.0;

        return usedMemKB;
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
            while (!_kbhit()) {
                //wait for input
            }
            input = _getch();
            if (_kbhit()) {
                input2 = _getch();
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
        std::string options = "Example1\nQuit";
        while (opt != std::ranges::count(options, '\n')) {
            opt = DsaDebug::menueScreen(options, opt, "OPTIONS:\n----------------------\n", "\n----------------------");
            system("cls");

            switch (opt) {
            case 0:

                break;
            }
        }

    }
};


