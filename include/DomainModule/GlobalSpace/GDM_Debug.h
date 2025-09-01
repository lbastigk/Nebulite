/**
 * @file GDM_Debug.h
 * 
 * This file contains an DomainModule of the GlobalSpaceTree for debugging capabilities.
 */

#pragma once

#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/DomainModuleWrapper.h"

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
class Debug : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::GlobalSpace, Debug> {
public:
    using DomainModuleWrapper<Nebulite::Core::GlobalSpace, Debug>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //------------------------------------------
    // TEST: Binding a dummy function with a name that already exists in the subtree
    // This function exists just for testing purposes and is not meant to be used in production code.
    // Instead, it serves as a placeholder to demonstrate the binding mechanisms collision detection
    // Uncomment the bind in setupBindings() to test the collision detection.
    // The binary compilation will work, but execution will fail.
    Nebulite::Constants::ERROR_TYPE set(int argc, char* argv[]) {
        // Binding a function with the name "set" is not allowed as it already exists in the subtree JSONTree
        return Nebulite::Constants::ERROR_TYPE::NONE;
    }

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
     */
    Nebulite::Constants::ERROR_TYPE errorlog(int argc, char* argv[]);

    /**
     * @brief Prints the global document to the console.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE printGlobal(int argc, char* argv[]);

    /**
     * @brief Prints the current state of the renderer to the console.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE printState(int argc, char* argv[]);

    /**
     * @brief Logs the global document to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "global.log.jsonc" if no name was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE logGlobal(int argc, char* argv[]);

    /**
     * @brief Logs the current state of the renderer to a file.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <filenames>. Logs to all provided files.
     * Default is "state.log.jsonc" if no name was provided
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE logState(int argc, char* argv[]);

    /**
     * @brief Attach a command to the always-taskqueue that is executed on each tick.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <command>. The command to attach.
     * @return Potential errors that occured on command execution
     * 
     * @todo Move to GDM_General
     */
    Nebulite::Constants::ERROR_TYPE always(int argc, char* argv[]);

    /**
     * @brief Clears the entire always-taskqueue.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     * 
     * @todo Move to GDM_General
     */
    Nebulite::Constants::ERROR_TYPE alwaysClear(int argc, char* argv[]);

    /**
     * @brief Logs a standard render object to a file: ./Resources/Renderobjects/standard.jsonc.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE render_object(int argc, char** argv);


    
    //------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Debug::errorlog,          "log",                      "Activate/Deactivate error logging: log <on/off>");
        bindFunction(&Debug::printGlobal,       "print-global",             "Print global document");
        bindFunction(&Debug::printState,        "print-state",              "Print current state");
        bindFunction(&Debug::logGlobal,         "log-global",               "Log global document: log-global [filename]");
        bindFunction(&Debug::logState,          "log-state",                "Log current state: log-state [filename]");
        bindFunction(&Debug::always,            "always",                   "Attach function to always run: always <command>");
        bindFunction(&Debug::alwaysClear,       "always-clear",             "Clear all always functions");
        bindFunction(&Debug::render_object,     "standard-render-object",   "Generates a standard render object at ./Resources/Renderobjects/standard.jsonc");

        //------------------------------------------
        // Example Bindings that will fail

        // TEST: Binding an already existing sub-function
        //bindFunction(&Debug::set, "set", "Dummy function to test binding with existing name in subtree");  // <- THIS WILL FAIL

        // TEST: Binding an already existing function
        //bindFunction(&Debug::set, "log", "Dummy function to test binding with existing name in own tree"); // <- THIS WILL FAIL
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