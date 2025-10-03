/**
 * @file GDM_Debug.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace for debugging capabilities.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Debug
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Debug) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    /**
     * @brief Dummy function for testing function definition collision detection of bindFunction.
     * 
     * This function is intentionally left blank to test collision detection.
     * It is not meant to be bound in production code.
     * Instead, it serves as a placeholder to demonstrate the binding mechanisms collision detection
     * Uncomment the bind in setupBindings() to test the collision detection.
     * The binary compilation will work, but execution will fail.
     */
    Nebulite::Constants::Error set(int argc, char* argv[]) {
        // Binding a function with the name "set" is not allowed 
        // as it already exists in the inherited domain JSON
        return Nebulite::Constants::ErrorTable::NONE();
    }
    std::string set_desc = R"(If you see this message, function collision detection is NOT working!
    With this function, GlobalSpace should not be able to initialize, as the function "set" already exists in the inherited domain JSON.
    )";

    //------------------------------------------
    // Available Functions

    /**
     * @brief Activates or deactivates error logging to a file
     * 
     * This function toggles the error logging status between the standard error output
     * and a specified log file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are "on" or "off"
     * @return Potential errors that occured on command execution
     * 
     * @todo: errorlog on causes crash with wine
     * wine: Unhandled page fault on write access to 0000000000000000 at address 0000000140167A65 (thread 0110), starting debugger...
     */
    Nebulite::Constants::Error errorlog(int argc, char* argv[]);
    std::string errorlog_desc = R"(Activates or deactivates error logging to a file.
    Usage: errorlog <on/off>
    - on:  Activates error logging to 'error.log' in the working directory.
    - off: Deactivates error logging, reverting to standard error output.
    Note: Ensure you have write permissions in the working directory when activating error logging.
    )";

    /**
     * @brief Clears the console screen.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error clearConsole(int argc, char* argv[]);
    std::string clearConsole_desc = R"(Clears the console screen.
    Usage: clear
    Note: This function attempts to clear the console screen using system-specific commands.
            It may not work in all environments or IDEs.
    )";

    /**
     * @brief Logs the global document to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "global.log.jsonc" if no name was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error log_global(int argc, char* argv[]);
    std::string log_global_desc = R"(Logs the global document to a file.
    Usage: log global [<filenames>...]
    - <filenames>: Optional. One or more filenames to log the global document to.
                    If no filenames are provided, defaults to 'global.log.jsonc'.
    )";

    /**
     * @brief Logs the current state of the renderer to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "state.log.jsonc" if no name was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error log_state(int argc, char* argv[]);
    std::string log_state_desc = R"(Logs the current state of the renderer to a file.
    Usage: log state [<filenames>...]
    - <filenames>: Optional. One or more filenames to log the renderer state to.
                    If no filenames are provided, defaults to 'state.log.jsonc'.
    )";

    /**
     * @brief Crashes the program, useful for checking 
     * if the testing suite can catch crashes.
     * 
     * @param argc The argument count
     * @param argv The argument vector: The type of crash: [segfault/abort/terminate/throw]
     * Default is segfault if no argument was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error crash(int argc, char** argv);
    std::string crash_desc = R"(Crashes the program, useful for checking if the testing suite can catch crashes.
    Usage: crash [<type>]
    - <type>: Optional. The type of crash to induce. Options are:
        - segfault   : Causes a segmentation fault (default)
        - abort      : Calls std::abort()
        - terminate  : Calls std::terminate()
        - throw      : Throws an uncaught exception
    )";

    /**
     * @brief Echoes all arguments as string to the standard error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error error(int argc, char* argv[]);
    std::string error_desc = R"(Echoes all arguments as string to the standard error.
    Usage: error <string...>
    - <string...>: One or more strings to echo to the standard error.
    )";

    /**
     * @brief Returns a warning: a custom, noncritical error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error. 
     */
    Nebulite::Constants::Error warn(int argc, char* argv[]);
    std::string warn_desc = R"(Returns a warning: a custom, noncritical error.
    Usage: warn <string...>
    - <string...>: One or more strings to include in the warning message.
    )";

    /**
     * @brief Returns a critical error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error.
     */
    Nebulite::Constants::Error critical(int argc, char* argv[]);
    std::string critical_desc = R"(Returns a critical error.
    Usage: critical <string...>
    - <string...>: One or more strings to include in the critical error message.
    )";

    /**
     * @brief Waits for user input before continuing.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error waitForInput(int argc, char* argv[]);
    std::string waitForInput_desc = R"(Waits for user input before continuing.
    Usage: inputwait
    Note: This function pauses execution until the user presses Enter.
    )";

    /**
     * @brief Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error standardfile_renderobject(int argc, char** argv);
    std::string standardfile_renderobject_desc = R"(Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.
    Usage: standardfile renderobject
    Note: This function creates or overwrites the file 'standard.jsonc' in the './Resources/Renderobjects/' directory.
    )";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug){
        //------------------------------------------
        // Binding functions to the FuncTree
        bindFunction(&Debug::errorlog,          "errorlog",                 errorlog_desc);
        bindFunction(&Debug::clearConsole,      "clear",                    clearConsole_desc);
        bindFunction(&Debug::error,             "error",                    error_desc);
        bindFunction(&Debug::crash,             "crash",                    crash_desc);
        bindFunction(&Debug::warn,              "warn",                     warn_desc);
        bindFunction(&Debug::critical,          "critical",                 critical_desc);
        bindFunction(&Debug::waitForInput,      "inputwait",                waitForInput_desc);

        bindSubtree("log", "Functions to log various data to files");
        bindFunction(&Debug::log_global,        "log global",               log_global_desc);
        bindFunction(&Debug::log_state,         "log state",                log_state_desc);

        bindSubtree("standardfile", "Functions to generate standard files");
        bindFunction(&Debug::standardfile_renderobject,     "standardfile renderobject",   standardfile_renderobject_desc);

        //------------------------------------------
        // Example Bindings that will fail

        // TEST: Binding an already existing sub-function
        //bindFunction(&Debug::set, "set", set_desc);  // <- THIS WILL FAIL, as "set" already exists in the inherited domain JSON

        // TEST: Binding an already existing function
        //bindFunction(&Debug::error, "error", error_desc); // <- THIS WILL FAIL, as "error" already exists in GlobalSpace
    }

private:
    std::streambuf* originalCerrBuf = nullptr;
    std::unique_ptr<std::ofstream> errorFile;
    
    // Current status of error logging
    // false : logging to cerr
    // true  : logging to file
    bool errorLogStatus = false;
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite