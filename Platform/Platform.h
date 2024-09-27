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

#include <windows.h> // for ShellExecute

#elif defined(__linux__) // Linux

#include <unistd.h> // for exec and fork
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#endif

class Platform{
public:
    static int hasKeyBoardInput();

    static void clearScreen();

    static bool openFile(std::string fullPath);

    static int getCharacter();

    static double getMemoryUsagekB();
private:
};