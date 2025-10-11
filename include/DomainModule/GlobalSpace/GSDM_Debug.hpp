/**
 * @file GSDM_Debug.hpp
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
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

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
    static const std::string set_name;
    static const std::string set_desc;

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
    static const std::string errorlog_name;
    static const std::string errorlog_desc;

    /**
     * @brief Clears the console screen.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error clearConsole(int argc, char* argv[]);
    static const std::string clearConsole_name;
    static const std::string clearConsole_desc;

    /**
     * @brief Logs the global document to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "global.log.jsonc" if no name was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error log_global(int argc, char* argv[]);
    static const std::string log_global_name;
    static const std::string log_global_desc;

    /**
     * @brief Logs the current state of the renderer to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "state.log.jsonc" if no name was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error log_state(int argc, char* argv[]);
    static const std::string log_state_name;
    static const std::string log_state_desc;

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
    static const std::string crash_name;
    static const std::string crash_desc;

    /**
     * @brief Echoes all arguments as string to the standard error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error error(int argc, char* argv[]);
    static const std::string error_name;
    static const std::string error_desc;

    /**
     * @brief Returns a warning: a custom, noncritical error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error. 
     */
    Nebulite::Constants::Error warn(int argc, char* argv[]);
    static const std::string warn_name;
    static const std::string warn_desc;

    /**
     * @brief Returns a critical error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error.
     */
    Nebulite::Constants::Error critical(int argc, char* argv[]);
    static const std::string critical_name;
    static const std::string critical_desc;

    /**
     * @brief Waits for user input before continuing.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error waitForInput(int argc, char* argv[]);
    static const std::string waitForInput_name;
    static const std::string waitForInput_desc;

    /**
     * @brief Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error standardfile_renderobject(int argc, char** argv);
    static const std::string standardfile_renderobject_name;
    static const std::string standardfile_renderobject_desc;

    //------------------------------------------
    // Subtree names
    static const std::string log_name;
    static const std::string log_desc;

    static const std::string standardfile_name;
    static const std::string standardfile_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Debug){
        //------------------------------------------
        // Binding functions to the FuncTree
        bindFunction(&Debug::errorlog,          errorlog_name,                 &errorlog_desc);
        bindFunction(&Debug::clearConsole,      clearConsole_name,             &clearConsole_desc);
        bindFunction(&Debug::error,             error_name,                    &error_desc);
        bindFunction(&Debug::crash,             crash_name,                    &crash_desc);
        bindFunction(&Debug::warn,              warn_name,                     &warn_desc);
        bindFunction(&Debug::critical,          critical_name,                 &critical_desc);
        bindFunction(&Debug::waitForInput,      waitForInput_name,             &waitForInput_desc);

        bindSubtree(log_name, &log_desc);
        bindFunction(&Debug::log_global,        log_global_name,               &log_global_desc);
        bindFunction(&Debug::log_state,         log_state_name,                &log_state_desc);

        bindSubtree(standardfile_name, &standardfile_desc);
        bindFunction(&Debug::standardfile_renderobject,     standardfile_renderobject_name,   &standardfile_renderobject_desc);

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