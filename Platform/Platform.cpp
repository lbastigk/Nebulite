// Platform depending functions
#include "Platform.h"

//Function definitions
#ifdef _WIN32 // Windows

    void Platform::flushKeyboardInput() {
        // Get the handle to the standard input
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        
        // Flush the input buffer
        FlushConsoleInputBuffer(hInput);
    }

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

    void Platform::flushKeyboardInput() {
        // tcflush requires a file descriptor, we use STDIN_FILENO for standard input
        tcflush(STDIN_FILENO, TCIFLUSH);
    }

    static struct termios old, current;
    /*
    bool Platform::hasKeyBoardInputOld(){
        
        termios term;
        tcgetattr(0, &term);

        termios term2 = term;
        term2.c_lflag &= ~ICANON;
        tcsetattr(0, TCSANOW, &term2);

        int byteswaiting;
        ioctl(0, FIONREAD, &byteswaiting);

        tcsetattr(0, TCSANOW, &term);

        return byteswaiting > 0;


    }
    */

    //TODO: does not work with enter or space
    bool Platform::hasKeyBoardInput() {
        // Save the current terminal settings
        struct termios oldt;
        tcgetattr(STDIN_FILENO, &oldt);

        // Modify terminal settings for non-canonical mode (disable line buffering)
        struct termios newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        // Check if input is available
        int byteswaiting = 0;
        ioctl(STDIN_FILENO, FIONREAD, &byteswaiting);

        // Restore the old terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        // Return true if there are bytes waiting to be read
        return byteswaiting > 0;
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

    void Platform::putCharacter(int character) {
        // Buffer to hold the character bytes
        char buf[4] = {0};  // A maximum of 4 bytes to handle UTF-8 encoded characters

        // Convert the integer into its byte representation
        buf[0] = (character >> 8*0) & 0xFF;
        buf[1] = (character >> 8*1) & 0xFF;
        buf[2] = (character >> 8*2) & 0xFF;
        buf[3] = (character >> 8*3) & 0xFF;

        // Determine the number of bytes to write
        int numBytes = 0;
        if (buf[3]) numBytes = 4;
        else if (buf[2]) numBytes = 3;
        else if (buf[1]) numBytes = 2;
        else numBytes = 1;

        // Write the character bytes to stdout
        write(1, buf, numBytes);
    }

    std::string Platform::vectorToString(const std::vector<int>& characterVector) {
    std::string result;
    
    // Iterate through each integer in the vector
    for (int character : characterVector) {
        char buf[4] = {0}; // Buffer to hold the bytes of each character
        
        // Extract the bytes from the integer
        buf[0] = character & 0xFF;
        buf[1] = (character >> 8) & 0xFF;
        buf[2] = (character >> 16) & 0xFF;
        buf[3] = (character >> 24) & 0xFF;
        
        // Determine how many bytes are needed to represent the character
        int numBytes = 1;
        if (buf[1] != 0) numBytes = 2;
        if (buf[2] != 0) numBytes = 3;
        if (buf[3] != 0) numBytes = 4;
        
        // Append the character(s) to the result string
        result.append(buf, numBytes);
    }

    return result;
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