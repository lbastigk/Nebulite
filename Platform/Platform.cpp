// Platform depending functions
#include "Platform.h"



//Function definitions
#ifdef _WIN32 // Windows



    int Platform::hasKeyBoardInput(){
        return _kbhit();
    }

    void Platform::clearScreen(){
        system("cls");
    }

    bool Platform::openFile(std::string fullPath){
        HINSTANCE result = ShellExecute(nullptr, L"open", std::wstring(fullPath.begin(), fullPath.end()).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return (intptr_t)result > 32;
    }

    char Platform::getCharacter(){
        return _getch();
    }

    double Platform::getMemoryUsagekB() {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

        // Convert bytes to kilobytes as double
        double usedMemKB = static_cast<double>(pmc.PrivateUsage) / 1024.0;

        return usedMemKB;
    }

#elif defined(__linux__) // Linux



    static struct termios old, current;
    
    int Platform::hasKeyBoardInput(){
        struct termios oldt, newt;
        int ch;
        int oldf;
        
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        
        ch = getchar();
        
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        
        if(ch != EOF)
        {
            ungetc(ch, stdin);
            return 1;
        }
        
        return 0;
    }

    void Platform::clearScreen(){
        system("clear");
    }

    bool Platform::openFile(std::string fullPath) {
        // Using fork and exec to run a program
        pid_t pid = fork();
        if (pid == 0) { // Child process
            execlp("xdg-open", "xdg-open", fullPath.c_str(), nullptr);
            // If execlp fails
            std::cerr << "Failed to open file: " << fullPath << std::endl;
            exit(EXIT_FAILURE);
        } else if (pid > 0) { // Parent process
            int status;
            waitpid(pid, &status, 0); // Wait for the child process to complete
            return WIFEXITED(status);
        } else {
            std::cerr << "Failed to fork process" << std::endl;
            return false;
        }
    }



    int Platform::getCharacter() {
        // Disable echo of pressed characters while function is active
        // Flush stdin at function call so rest of buffer from previous chars isn't used

        char buf[4] = {0}; // Buffer to hold 2 characters + null terminator
        struct termios old = {0};
        
        fflush(stdout);
        if(tcgetattr(0, &old) < 0){
            std::cout << "Error tcgetattr()";
            perror("tcgetattr()");
        }
            
        
        struct termios newattr = old;
        newattr.c_lflag &= ~ICANON; // Disable canonical mode
        newattr.c_lflag &= ~ECHO;   // Disable echo
        newattr.c_cc[VMIN] = 1;      // Minimum number of characters to read
        newattr.c_cc[VTIME] = 0;     // No timeout
        
        // Set the new terminal attributes
        if(tcsetattr(0, TCSANOW, &newattr) < 0){
            std::cout << "Error tcsetattr ICANON";
            perror("tcsetattr ICANON");
        } 

        // Read the first character
        read(0, &buf[0], 4);

        // Restore the old terminal attributes
        if(tcsetattr(0, TCSANOW, &old) < 0){
            std::cout << "Error tcsetattr ~ICANON";
            perror("tcsetattr ~ICANON");
        }

        int val = 0;
        val += (int)buf[0] << 8*0;
        val += (int)buf[1] << 8*1;
        val += (int)buf[2] << 8*2;
        val += (int)buf[3] << 8*4;

        return val;
    }

    double Platform::getMemoryUsagekB() {
        std::ifstream statmFile("/proc/self/statm");
        if (!statmFile.is_open()) {
            std::cerr << "Could not open /proc/self/statm" << std::endl;
            return -1; // Return -1 to indicate an error
        }

        unsigned long size; // Memory size in pages
        unsigned long resident; // Resident set size in pages
        unsigned long shared; // Shared memory in pages
        unsigned long text; // Text (code) size in pages
        unsigned long lib; // Library size in pages
        unsigned long data; // Data + stack size in pages
        unsigned long dt; // Dirty pages size in pages

        statmFile >> size >> resident >> shared >> text >> lib >> data >> dt;

        // Close the file
        statmFile.close();

        // Get the page size
        long pageSize = sysconf(_SC_PAGESIZE); // Page size in bytes

        // Convert resident set size from pages to kilobytes
        double usedMemKB = static_cast<double>(resident * pageSize) / 1024.0;

        return usedMemKB;
    }



#else
      
#endif