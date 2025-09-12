/**
 * @file GDM_Debug.hpp
 * 
 * This file contains the DomainModule of the GlobalSpaceTree for debugging capabilities.
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
class Debug : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
public:
    /**
     * @brief Overridden update function.
     */
    void update();

    //------------------------------------------
    /**
     * @brief Dummy function for testing function definition collision detection of FuncTree.
     * 
     * This function is intentionally left blank to test collision detection.
     * It is not meant to be bound in production code.
     * Instead, it serves as a placeholder to demonstrate the binding mechanisms collision detection
     * Uncomment the bind in setupBindings() to test the collision detection.
     * The binary compilation will work, but execution will fail.
     */
    Nebulite::Constants::ERROR_TYPE set(int argc, char* argv[]) {
        // Binding a function with the name "set" is not allowed as it already exists in the inherited FuncTree JSONTree
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
     * 
     * @todo: log on causes crash with wine
     * wine: Unhandled page fault on write access to 0000000000000000 at address 0000000140167A65 (thread 0110), starting debugger...
     */
    Nebulite::Constants::ERROR_TYPE errorlog(int argc, char* argv[]);

    /**
     * @brief Clears the console screen.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE clearConsole(int argc, char* argv[]);

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
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Debug(std::string moduleName, Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        //------------------------------------------
        // Binding functions to the FuncTree
        bindFunction(&Debug::errorlog,          "log",                      "Activate/Deactivate error logging: log <on/off>");
        bindFunction(&Debug::clearConsole,      "clear",                    "Clear console");
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
        //bindFunction(&Debug::set, "set", "Dummy function to test binding with existing name in inherited FuncTree");  // <- THIS WILL FAIL

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