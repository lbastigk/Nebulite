#include "DomainModule/GlobalSpace/GSDM_Debug.hpp"
#include "Core/GlobalSpace.hpp"       // Global Space for Nebulite

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <unistd.h>   // isatty
  #include <cstdio>
#endif

namespace Nebulite::DomainModule::GlobalSpace {

const std::string Debug::log_name = "log";
const std::string Debug::log_desc = R"(Functions to log various data to files)";

const std::string Debug::standardfile_name = "standardfile";
const std::string Debug::standardfile_desc = R"(Functions to generate standard files)";

//------------------------------------------
// Update
Nebulite::Constants::Error Debug::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Nebulite::Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

Nebulite::Constants::Error Debug::log_global([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
    std::string serialized = domain->getDoc()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Nebulite::Utility::FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        Nebulite::Utility::FileManagement::WriteFile("global.log.jsonc",serialized);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::log_global_name = "log global";
const std::string Debug::log_global_desc = R"(Logs the global document to a file.

Usage: log global [<filenames>...]

- <filenames>: Optional. One or more filenames to log the global document to.
                If no filenames are provided, defaults to 'global.log.jsonc'.
)";

Nebulite::Constants::Error Debug::log_state([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
    std::string serialized = domain->getRenderer()->serialize();
    if (argc>1){
        for(int i=1; i < argc; i++){
            Nebulite::Utility::FileManagement::WriteFile(argv[i],serialized);
        }
    }
    else{
        Nebulite::Utility::FileManagement::WriteFile("state.log.jsonc",serialized);
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::log_state_name = "log state";
const std::string Debug::log_state_desc = R"(Logs the current state of the renderer to a file.

Usage: log state [<filenames>...]

- <filenames>: Optional. One or more filenames to log the renderer state to.
                If no filenames are provided, defaults to 'state.log.jsonc'.
)";

Nebulite::Constants::Error Debug::standardfile_renderobject([[maybe_unused]] int argc, [[maybe_unused]] char** argv){
    Nebulite::Core::RenderObject ro(domain);
    Nebulite::Utility::FileManagement::WriteFile("./Resources/Renderobjects/standard.jsonc",ro.serialize());
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::standardfile_renderobject_name = "standardfile renderobject";
const std::string Debug::standardfile_renderobject_desc = R"(Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.

Usage: standardfile renderobject

Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.
)";

Nebulite::Constants::Error Debug::errorlog([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
    // Initialize the error logging buffer
    if(!originalCerrBuf) {
        // Handle the case where originalCerrBuf is not set
        originalCerrBuf = std::cerr.rdbuf();
    }

    if(argc == 2){
        if(!strcmp(argv[1], "on")){
            if(!errorLogStatus){
                try {
                    // Create ofstream only when needed (lazy initialization)
                    if (!errorFile) {
                        errorFile = std::make_unique<std::ofstream>();
                    }
                    
                    // Log errors in separate file
                    errorFile->open("errors.log");
                    if (!(*errorFile)) {
                        std::cerr << "Failed to open error file." << std::endl;
                        return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                    }
                    
                    originalCerrBuf = std::cerr.rdbuf(); // Store the original cerr buffer
                    std::cerr.rdbuf(errorFile->rdbuf()); // Redirect to file
                    errorLogStatus = true;
                } catch (const std::exception& e) {
                    std::cerr << "Failed to create error log: " << e.what() << std::endl;
                    return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                } catch (...) {
                    std::cerr << "Failed to create error log: unknown error" << std::endl;
                    return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE();
                }
            }
        }
        else if (!strcmp(argv[1], "off")){
            if(errorLogStatus){
                // Close error log
                std::cerr.flush();                    // Flush before restoring
                std::cerr.rdbuf(originalCerrBuf);     // Restore the original buffer
                
                if (errorFile) {
                    errorFile->close();
                    // Keep the unique_ptr for potential reuse
                }
                
                errorLogStatus = false;
            }
        } 
    }
    else{
        if(argc > 2){
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
        }
        else{
            return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
        }
    }
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::errorlog_name = "errorlog";
const std::string Debug::errorlog_desc = R"(Activates or deactivates error logging to a file.

Usage: errorlog <on/off>

- on:  Activates error logging to 'error.log' in the working directory.
- off: Deactivates error logging, reverting to standard error output.
Note: Ensure you have write permissions in the working directory when activating error logging.
)";

inline void clear_screen()
{
#if defined(_WIN32)
    // Use Win32 API to clear the console buffer and move cursor to home (0,0).
    HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStd == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hStd, &csbi)) return;

    DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written = 0;
    COORD home = { 0, 0 };

    // Fill the entire buffer with spaces
    FillConsoleOutputCharacterA(hStd, ' ', cells, home, &written);
    // Reset attributes
    FillConsoleOutputAttribute(hStd, csbi.wAttributes, cells, home, &written);
    // Move the cursor home
    SetConsoleCursorPosition(hStd, home);

#else
    // If stdout is a terminal, use ANSI escapes to clear the screen and move cursor to top-left.
    if (isatty(fileno(stdout))) {
        // ESC[2J clears screen; ESC[H moves cursor to 1;1
        std::cout << "\x1b[2J\x1b[H" << std::flush;
    } else {
        // Not a TTY (redirected output). We can optionally print newlines or do nothing.
        // Printing newlines keeps behavior similar to clearing for plain output.
        std::cout << std::endl;
    }
#endif
}

Nebulite::Constants::Error Debug::clearConsole(int argc, [[maybe_unused]] char* argv[]){
    if (argc > 1) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    clear_screen();
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::clearConsole_name = "clear";
const std::string Debug::clearConsole_desc = R"(Clears the console screen.

Usage: clear

Note: This function attempts to clear the console screen using system-specific commands.
        It may not work in all environments or IDEs.
)";

Nebulite::Constants::Error Debug::crash(int argc, char** argv) {
    // If an argument is provided, use it to select crash type
    if (argc > 1 && argv[1]) {
        std::string crashType = argv[1];
        if (crashType == "segfault") {
            // Cause a segmentation fault
            int* p = nullptr;
            *p = 42;
        } else if (crashType == "abort") {
            // Abort the program
            std::abort();
        } else if (crashType == "terminate") {
            // Terminate with std::terminate
            std::terminate();
        } else if (crashType == "throw") {
            // Throw an uncaught exception
            throw std::runtime_error("Intentional crash: uncaught exception");
        } else {
            std::cerr << "Unknown crash type requested: " << crashType << std::endl;
            std::cerr << "Defaulting to segmentation fault" << std::endl;
        }
    } else {
        // Default: segmentation fault
        int* p = nullptr;
        *p = 42;
    }
    // Should never reach here
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::crash_name = "crash";
const std::string Debug::crash_desc = R"(Crashes the program, useful for checking if the testing suite can catch crashes.

Usage: crash [<type>]

- <type>: Optional. The type of crash to induce. Options are:
    - segfault   : Causes a segmentation fault (default)
    - abort      : Calls std::abort()
    - terminate  : Calls std::terminate()
    - throw      : Throws an uncaught exception
)";

Nebulite::Constants::Error Debug::error([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::cerr << argv[i];
        if (i < argc - 1) {
            std::cerr << " ";
        }
    }
    std::cerr << std::endl;

    // No further error to return
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::error_name = "error";
const std::string Debug::error_desc = R"(Echoes all arguments as string to the standard error.

Usage: error <string...>

- <string...>: One or more strings to echo to the standard error.
)";

Nebulite::Constants::Error Debug::warn([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string str = "";
    for(int i = 1; i < argc; ++i){
        str += argv[i];
        if(i < argc - 1){
            str += " ";
        }
    }
    return Nebulite::Constants::ErrorTable::addError(str, Nebulite::Constants::Error::NON_CRITICAL);
}
const std::string Debug::warn_name = "warn";
const std::string Debug::warn_desc = R"(Returns a warning: a custom, noncritical error.

Usage: warn <string>

- <string>: The warning message.
)";


Nebulite::Constants::Error Debug::critical([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    std::string str = "";
    for(int i = 1; i < argc; ++i){
        str += argv[i];
        if(i < argc - 1){
            str += " ";
        }
    }
    return Nebulite::Constants::ErrorTable::addError(str, Nebulite::Constants::Error::CRITICAL);
}
const std::string Debug::critical_name = "critical";
const std::string Debug::critical_desc = R"(Returns a critical error.

Usage: critical <string>

- <string>: The critical error message.
)";

Nebulite::Constants::Error Debug::waitForInput([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]){
    if (argc > 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    std::string message = "Press Enter to continue...";
    if (argc == 2){
        // Use the provided prompt as message
        message = argv[1];
    }
    std::cout << message << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Debug::waitForInput_name = "inputwait";
const std::string Debug::waitForInput_desc = R"(Waits for user input before continuing.

Usage: inputwait [prompt]

Note: This function pauses execution until the user presses Enter.
)";

} // namespace Nebulite::DomainModule::GlobalSpace