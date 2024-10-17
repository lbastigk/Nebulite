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
#include <vector>

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

#include <sys/ioctl.h>
#include <termios.h>

#endif

class Platform{
public:
    //static void init();

    static void flushKeyboardInput();

    static bool hasKeyBoardInput();

    static void clearScreen();

    static bool openFile(std::string fullPath);

    static int getCharacter();

    static void putCharacter(int character);

    static std::string vectorToString(const std::vector<int>& characterVector);

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
            // Special
            const static int Enter = 10;
            const static int Backspace = 127;
            const static int Space = 32;
            const static int Tab = 9;
            // Numbers
            const static int NO_0 = 48;
            const static int NO_1 = 49;
            const static int NO_2 = 50;
            const static int NO_3 = 51;
            const static int NO_4 = 52;
            const static int NO_5 = 53;
            const static int NO_6 = 54;
            const static int NO_7 = 55;
            const static int NO_8 = 56;
            const static int NO_9 = 57;
            // Arrows
            const static int arrowUp = 4283163;
            const static int arrowDown = 4348699;
            const static int arrowLeft = 4479771;
            const static int arrowRight = 4414235;
            // First row
            const static int Q = 113;
            const static int W = 119;
            const static int E = 101;
            const static int R = 114;
            const static int T = 116;
            const static int Z = 122;
            const static int U = 117;
            const static int I = 105;
            const static int O = 111;
            const static int P = 112;
            const static int Ü = -17469;
            // Second row
            const static int A = 97;
            const static int S = 115;
            const static int D = 100;
            const static int F = 102;
            const static int G = 103;
            const static int H = 104;
            const static int J = 106;
            const static int K = 107;
            const static int L = 108;
            const static int Ö = -19005;
            const static int Ä = -23613;
            // Third row
            const static int Y = 121;
            const static int X = 120;
            const static int C = 99;
            const static int V = 118;
            const static int B = 98;
            const static int N = 110;
            const static int M = 109;
        #endif
        
    };

    #ifdef WIN32
        static const bool hasDefaultEcho = false;
    #elif defined(__linux__)
        static const bool hasDefaultEcho = true;
    #endif
    
private:

};