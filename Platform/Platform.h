#pragma once

//-----------------------------------------
//Includes, general
#include <iostream>
#include <fstream> 
#include <cstdlib> // for system()
#include <string>
#include <thread>
#include <optional>
#include <variant>
#include <locale>
#include <codecvt>

//-----------------------------------------
//Platform depending includes
#ifdef _WIN32

#define FILESYSTEM_SEPARATOR '\\'

#include <windows.h> // for ShellExecute

#elif defined(__linux__) // Linux

#define FILESYSTEM_SEPARATOR '/'

#include <unistd.h> // for exec and fork
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#endif

class Platform{
public:
    static void init();

    static int hasKeyBoardInput();

    static void clearScreen();

    static bool openFile(std::string fullPath);

    static int getCharacter();

    static double getMemoryUsagekB();

    class KeyPress{
    public:
        #ifdef _WIN32
            const static int Enter = 0;
            const static int arrowUp = 0;
            const static int arrowDown = 0;
            const static int arrowLeft = 0;
            const static int arrowRight = 0;
        #elif defined(__linux__)
            const static int Enter = 10;
            const static int arrowUp = 4283163;
            const static int arrowDown = 4348699;
            const static int arrowLeft = 4479771;
            const static int arrowRight = 4414235;
        #endif
        
    };
private:

};