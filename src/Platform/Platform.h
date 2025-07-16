/*
 * Platform.h
 * ----------
 * Provides a cross-platform abstraction layer for system-dependent functionality
 * such as screen clearing, file opening, character input/output, and memory usage.
 * 
 * This class uses conditional compilation to include the appropriate headers and 
 * define platform-specific constants and functions for Windows and Linux.
 *
 * Platform-based functions:
 *   - clearScreen(): Clears the terminal or console screen using the appropriate
 *     system call for the current platform.
 *   - openFile(std::string fullPath): Opens a file or URL with the default system
 *     application (e.g., ShellExecute on Windows, xdg-open on Linux).
 *   - getCharacter(): Reads a single character from the terminal without waiting
 *     for a newline, using _getch() on Windows or termios on Linux.
 *   - putCharacter(int character): Outputs a single character to the terminal.
 *   - vectorToString(const std::vector<int>&): Converts a vector of character codes
 *     to a string.
 *   - getMemoryUsagekB(): Returns the current process's memory usage in kilobytes,
 *     using platform-specific APIs (e.g., psapi.h on Windows, /proc/self/statm on Linux).
 *
 * Platform-specific constants:
 *   - FILESYSTEM_SEPARATOR: Set to '\\' on Windows and '/' on Linux for path handling.
 *   - hasDefaultEcho: Indicates whether the terminal echoes input by default
 *     (false on Windows, true on Linux).
 *
 * All functions are static and intended to be used without instantiating the Platform class.
 */

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

    static int clearScreen();

    static bool openFile(std::string fullPath);

    static int getCharacter();

    static void putCharacter(int character);

    static std::string vectorToString(const std::vector<int>& characterVector);

    static double getMemoryUsagekB();

    #ifdef WIN32
        static const bool hasDefaultEcho = false;
    #elif defined(__linux__)
        static const bool hasDefaultEcho = true;
    #endif
    
private:

};