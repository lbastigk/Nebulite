/**
 * @file Debug.hpp
 */

#ifndef NEBULITE_GSDM_DEBUG_HPP
#define NEBULITE_GSDM_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library
#include <fstream>
#include <memory>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"


//------------------------------------------
// Forward declarations
namespace Nebulite::Core{
    class GlobalSpace; // Forward declaration of domain class GlobalSpace
}   // namespace Nebulite::Core


//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace{
/**
 * @class Nebulite::DomainModule::GlobalSpace::Debug
 * @brief DomainModule for debugging capabilities within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Debug){
public:
    /**
     * @brief Override of update.
     */
    Constants::Error update() override;

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
     * @return Potential errors that occurred on command execution
     * 
     * @todo: errorlog on causes crash with wine
     * wine: Unhandled page fault on write access to 0000000000000000 at address 0000000140167A65 (thread 0110), starting debugger...
     */
    Constants::Error errorlog(int argc,  char** argv);
    static std::string const errorlog_name;
    static std::string const errorlog_desc;

    /**
     * @brief Clears the console screen.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error clearConsole(int argc,  char** argv);
    static std::string const clearConsole_name;
    static std::string const clearConsole_desc;

    /**
     * @brief Logs the global document to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "global.log.jsonc" if no name was provided
     * @return Potential errors that occurred on command execution
     */
    Constants::Error log_global(int argc,  char** argv);
    static std::string const log_global_name;
    static std::string const log_global_desc;

    /**
     * @brief Logs the current state of the renderer to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "state.log.jsonc" if no name was provided
     * @return Potential errors that occurred on command execution
     */
    Constants::Error log_state(int argc,  char** argv);
    static std::string const log_state_name;
    static std::string const log_state_desc;

    /**
     * @brief Crashes the program, useful for checking 
     * if the testing suite can catch crashes.
     * 
     * @param argc The argument count
     * @param argv The argument vector: The type of crash: [segfault/abort/terminate/throw]
     * Default is segfault if no argument was provided
     * @return Potential errors that occurred on command execution
     */
    Constants::Error crash(int argc,  char** argv);
    static std::string const crash_name;
    static std::string const crash_desc;

    /**
     * @brief Echoes all arguments as string to the standard error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occurred on command execution
     */
    Constants::Error error(int argc,  char** argv);
    static std::string const error_name;
    static std::string const error_desc;

    /**
     * @brief Returns a warning: a custom, noncritical error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error. 
     */
    Constants::Error warn(int argc,  char** argv);
    static std::string const warn_name;
    static std::string const warn_desc;

    /**
     * @brief Returns a critical error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error.
     */
    Constants::Error critical(int argc,  char** argv);
    static std::string const critical_name;
    static std::string const critical_desc;

    /**
     * @brief Waits for user input before continuing.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error waitForInput(int argc,  char** argv);
    static std::string const waitForInput_name;
    static std::string const waitForInput_desc;

    /**
     * @brief Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error standardfile_renderobject(int argc,  char** argv);
    static std::string const standardfile_renderobject_name;
    static std::string const standardfile_renderobject_desc;

    //------------------------------------------
    // Category names
    static std::string const log_name;
    static std::string const log_desc;

    static std::string const standardfile_name;
    static std::string const standardfile_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug){
        //------------------------------------------
        // Show platform info
        setupPlatformInfo();

        //------------------------------------------
        // Binding functions to the FuncTree
        bindFunction(&Debug::errorlog,          errorlog_name,                 &errorlog_desc);
        bindFunction(&Debug::clearConsole,      clearConsole_name,             &clearConsole_desc);
        bindFunction(&Debug::error,             error_name,                    &error_desc);
        bindFunction(&Debug::crash,             crash_name,                    &crash_desc);
        bindFunction(&Debug::warn,              warn_name,                     &warn_desc);
        bindFunction(&Debug::critical,          critical_name,                 &critical_desc);
        bindFunction(&Debug::waitForInput,      waitForInput_name,             &waitForInput_desc);

        (void)bindCategory(log_name, &log_desc);
        bindFunction(&Debug::log_global,        log_global_name,               &log_global_desc);
        bindFunction(&Debug::log_state,         log_state_name,                &log_state_desc);

        (void)bindCategory(standardfile_name, &standardfile_desc);
        bindFunction(&Debug::standardfile_renderobject,     standardfile_renderobject_name,   &standardfile_renderobject_desc);
    }

private:
    std::streambuf* originalCerrBuf = nullptr;
    std::unique_ptr<std::ofstream> errorFile;
    
    // Current status of error logging
    // false : logging to cerr
    // true  : logging to file
    bool errorLogStatus = false;

    /**
     * @brief Sets up platform information in the global document.
     */
    void setupPlatformInfo();
};
}   // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_DEBUG_HPP