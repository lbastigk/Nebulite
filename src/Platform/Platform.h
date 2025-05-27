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
#include <psapi.h>  //  to access memory functions
#include <conio.h>  //for _kbhit() and _getch()
#include <pthread.h>


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

    static void clearScreen();

    static bool openFile(std::string fullPath);

    static int getCharacter();

    static void putCharacter(int character);

    static std::string vectorToString(const std::vector<int>& characterVector);

    static double getMemoryUsagekB();

    class KeyPressOld{
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

    class KeyPress {
    public:
        #ifdef _WIN32
            // Special Keys
            const static int Enter = 13;             // ASCII for Enter key (Carriage Return)
            const static int Backspace = 8;          // ASCII for Backspace
            const static int Space = 32;             // ASCII for Space
            const static int Tab = 9;                // ASCII for Tab
            const static int Escape = 27;            // ASCII for Escape

            // Arrow Keys (Virtual Key codes)
            const static int arrowUp = 72;           // VK_UP virtual key code
            const static int arrowDown = 80;         // VK_DOWN virtual key code
            const static int arrowLeft = 75;         // VK_LEFT virtual key code
            const static int arrowRight = 77;        // VK_RIGHT virtual key code

            // Number keys (0-9)
            const static int NO_0 = 48;              // ASCII for '0'
            const static int NO_1 = 49;              // ASCII for '1'
            const static int NO_2 = 50;              // ASCII for '2'
            const static int NO_3 = 51;              // ASCII for '3'
            const static int NO_4 = 52;              // ASCII for '4'
            const static int NO_5 = 53;              // ASCII for '5'
            const static int NO_6 = 54;              // ASCII for '6'
            const static int NO_7 = 55;              // ASCII for '7'
            const static int NO_8 = 56;              // ASCII for '8'
            const static int NO_9 = 57;              // ASCII for '9'

            // Alphabetic keys (A-Z)
            const static int A = 65;                 // ASCII for 'A'
            const static int B = 66;                 // ASCII for 'B'
            const static int C = 67;                 // ASCII for 'C'
            const static int D = 68;                 // ASCII for 'D'
            const static int E = 69;                 // ASCII for 'E'
            const static int F = 70;                 // ASCII for 'F'
            const static int G = 71;                 // ASCII for 'G'
            const static int H = 72;                 // ASCII for 'H'
            const static int I = 73;                 // ASCII for 'I'
            const static int J = 74;                 // ASCII for 'J'
            const static int K = 75;                 // ASCII for 'K'
            const static int L = 76;                 // ASCII for 'L'
            const static int M = 77;                 // ASCII for 'M'
            const static int N = 78;                 // ASCII for 'N'
            const static int O = 79;                 // ASCII for 'O'
            const static int P = 80;                 // ASCII for 'P'
            const static int Q = 81;                 // ASCII for 'Q'
            const static int R = 82;                 // ASCII for 'R'
            const static int S = 83;                 // ASCII for 'S'
            const static int T = 84;                 // ASCII for 'T'
            const static int U = 85;                 // ASCII for 'U'
            const static int V = 86;                 // ASCII for 'V'
            const static int W = 87;                 // ASCII for 'W'
            const static int X = 88;                 // ASCII for 'X'
            const static int Y = 89;                 // ASCII for 'Y'
            const static int Z = 90;                 // ASCII for 'Z'

            // Function keys (F1-F12)
            const static int F1 = 112;               // VK_F1
            const static int F2 = 113;               // VK_F2
            const static int F3 = 114;               // VK_F3
            const static int F4 = 115;               // VK_F4
            const static int F5 = 116;               // VK_F5
            const static int F6 = 117;               // VK_F6
            const static int F7 = 118;               // VK_F7
            const static int F8 = 119;               // VK_F8
            const static int F9 = 120;               // VK_F9
            const static int F10 = 121;              // VK_F10
            const static int F11 = 122;              // VK_F11
            const static int F12 = 123;              // VK_F12

            // Modifier Keys
            const static int Shift = 16;             // VK_SHIFT
            const static int Ctrl = 17;              // VK_CONTROL
            const static int Alt = 18;               // VK_MENU (Alt)

            // Special Characters (using their ASCII codes)
            const static int Exclamation = 33;       // ASCII for '!'
            const static int DoubleQuote = 34;       // ASCII for '"'
            const static int Hash = 35;              // ASCII for '#'
            const static int Dollar = 36;            // ASCII for '$'
            const static int Percent = 37;           // ASCII for '%'
            const static int Ampersand = 38;         // ASCII for '&'
            const static int Apostrophe = 39;        // ASCII for '\''
            const static int LeftParenthesis = 40;   // ASCII for '('
            const static int RightParenthesis = 41;  // ASCII for ')'
            const static int Asterisk = 42;          // ASCII for '*'
            const static int Plus = 43;              // ASCII for '+'
            const static int Comma = 44;             // ASCII for ','
            const static int Minus = 45;             // ASCII for '-'
            const static int Period = 46;            // ASCII for '.'
            const static int Slash = 47;             // ASCII for '/'
            const static int Colon = 58;             // ASCII for ':'
            const static int Semicolon = 59;         // ASCII for ';'
            const static int LessThan = 60;          // ASCII for '<'
            const static int Equal = 61;             // ASCII for '='
            const static int GreaterThan = 62;       // ASCII for '>'
            const static int QuestionMark = 63;      // ASCII for '?'
            const static int At = 64;                // ASCII for '@'
            const static int LeftBracket = 91;       // ASCII for '['
            const static int Backslash = 92;         // ASCII for '\'
            const static int RightBracket = 93;      // ASCII for ']'
            const static int Caret = 94;             // ASCII for '^'
            const static int Underscore = 95;        // ASCII for '_'
            const static int GraveAccent = 96;       // ASCII for '`'

        #elif defined(__linux__)
            // Special Keys on Linux
            const static int Enter = 10;             // Newline (Enter key)
            const static int Backspace = 127;        // ASCII for Backspace
            const static int Space = 32;             // ASCII for Space
            const static int Tab = 9;                // ASCII for Tab

            // Number keys on Linux
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

            // Arrow keys on Linux (escape sequences or keycodes)
            const static int arrowUp = 4283163;      // Arrow Up sequence or key code
            const static int arrowDown = 4348699;    // Arrow Down sequence or key code
            const static int arrowLeft = 4479771;    // Arrow Left sequence or key code
            const static int arrowRight = 4414235;   // Arrow Right sequence or key code

            // Alphabet keys
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
            const static int Ü = -17469;             // Special character for certain locales

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