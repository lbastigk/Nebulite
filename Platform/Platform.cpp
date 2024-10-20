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

    /*
        int Platform::getCharacter() {
        // Disable echo of pressed characters while function is active
        char buf[4] = {0}; // Buffer for up to 4 bytes
        struct termios old = {0};
        
        fflush(stdout);
        if (tcgetattr(0, &old) < 0) {
            std::cout << "Error tcgetattr()";
            perror("tcgetattr()");
        }

        struct termios newattr = old;
        newattr.c_lflag &= ~ICANON; // Disable canonical mode
        newattr.c_lflag &= ~ECHO;   // Disable echo
        newattr.c_cc[VMIN] = 1;     // Minimum number of characters to read
        newattr.c_cc[VTIME] = 0;    // No timeout

        // Set the new terminal attributes
        if (tcsetattr(0, TCSANOW, &newattr) < 0) {
            std::cout << "Error tcsetattr ICANON";
            perror("tcsetattr ICANON");
        }

        // Read at least the first byte
        read(0, &buf[0], 1);

        // Determine the number of bytes for the character based on the first byte
        int numBytes = 1;
        unsigned char firstByte = buf[0];

        if ((firstByte & 0x80) == 0x00) {
            // Single-byte character (ASCII)
            numBytes = 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            // Two-byte character
            numBytes = 2;
        } else if ((firstByte & 0xF0) == 0xE0) {
            // Three-byte character
            numBytes = 3;
        } else if ((firstByte & 0xF8) == 0xF0) {
            // Four-byte character
            numBytes = 4;
        }

        // Read the remaining bytes of the multi-byte character, if any
        if (numBytes > 1) {
            read(0, &buf[1], numBytes - 1);
        }

        // Restore the old terminal attributes
        if (tcsetattr(0, TCSANOW, &old) < 0) {
            std::cout << "Error tcsetattr ~ICANON";
            perror("tcsetattr ~ICANON");
        }

        // Return the character as an integer, combining the bytes
        int val = 0;
        for (int i = 0; i < numBytes; ++i) {
            val += (int)((unsigned char)buf[i]) << (8 * i);
        }

        return val;
    }

    */

    int Platform::getCharacter() {
        // Buffer for up to 4 bytes (UTF-8 character handling)
        char buf[4] = {0};
        struct termios old = {0};
        
        // Get current terminal settings
        if (tcgetattr(0, &old) < 0) {
            std::cerr << "Error: tcgetattr()";
            perror("tcgetattr()");
            return -1;
        }

        // Modify terminal settings for non-canonical mode and no echo
        struct termios newattr = old;
        newattr.c_lflag &= ~(ICANON | ECHO);
        newattr.c_cc[VMIN] = 1;  // Minimum number of characters to read
        newattr.c_cc[VTIME] = 0; // No timeout

        // Apply the modified terminal settings
        if (tcsetattr(0, TCSANOW, &newattr) < 0) {
            std::cerr << "Error: tcsetattr ICANON";
            perror("tcsetattr ICANON");
            return -1;
        }

        // Set stdin (file descriptor 0) to non-blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        // Try to read the first byte (non-blocking)
        int result = read(0, &buf[0], 1);

        if (result == -1) {
            // If no input is available, return 0
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No key pressed
                tcsetattr(0, TCSANOW, &old);  // Restore original terminal settings
                fcntl(STDIN_FILENO, F_SETFL, flags);  // Restore blocking mode
                return 0;
            } else {
                std::cerr << "Error: read()";
                perror("read()");
                tcsetattr(0, TCSANOW, &old);  // Restore original terminal settings
                fcntl(STDIN_FILENO, F_SETFL, flags);  // Restore blocking mode
                return -1;
            }
        }

        // Determine the number of bytes for the character (handling UTF-8)
        int numBytes = 1;
        unsigned char firstByte = buf[0];

        if ((firstByte & 0x80) == 0x00) {
            // Single-byte character (ASCII)
            numBytes = 1;
        } else if ((firstByte & 0xE0) == 0xC0) {
            // Two-byte character
            numBytes = 2;
        } else if ((firstByte & 0xF0) == 0xE0) {
            // Three-byte character
            numBytes = 3;
        } else if ((firstByte & 0xF8) == 0xF0) {
            // Four-byte character
            numBytes = 4;
        }

        // Read the remaining bytes of the multi-byte character, if any
        if (numBytes > 1) {
            read(0, &buf[1], numBytes - 1);
        }

        // Restore original terminal settings
        if (tcsetattr(0, TCSANOW, &old) < 0) {
            std::cerr << "Error: tcsetattr ~ICANON";
            perror("tcsetattr ~ICANON");
            return -1;
        }

        // Restore stdin to blocking mode
        fcntl(STDIN_FILENO, F_SETFL, flags);

        // Combine the bytes into a single integer (character)
        int val = 0;
        for (int i = 0; i < numBytes; ++i) {
            val += (int)((unsigned char)buf[i]) << (8 * i);
        }

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