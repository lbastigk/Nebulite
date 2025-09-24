/**
 * @file GDM_General.hpp
 * 
 * This file contains the DomainModule of the GlobalSpace for general-purpose functions.
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
 * @class Nebulite::DomainModule::GlobalSpace::General
 * @brief DomainModule for general-purpose functions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, General) {
public:
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * @param argc The argument count
     * @param argv The argument vector: the string to evaluate
     * @return Potential errors that occured on command execution
     * 
     * Examples:
     * 
     * eval echo $(1+1)    outputs:    2.000000
     * eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
     */
    Nebulite::Constants::Error eval(int argc, char* argv[]);

    /**
     * @brief Exits the entire program
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error exitProgram(int argc, char* argv[]);

    /**
     * @brief Sets the waitCounter to the given value to halt all script tasks for a given amount of frames
     * 
     * @param argc The argument count
     * @param argv The argument vector: frame count to wait
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error wait(int argc, char* argv[]);

    /**
     * @brief Loads a task list from a file
     * 
     * @param argc The argument count
     * @param argv The argument vector: the filename to load
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error loadTaskList(int argc, char* argv[]);

    /**
     * @brief Executes a for-loop with a function call
     * 
     * @param argc The argument count
     * @param argv The argument vector: <var> <start> <end> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error forLoop(int argc, char* argv[]);

    /**
     * @brief Executes a block of code if a condition is true
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error ifCondition(int argc, char* argv[]);

    /**
     * @brief Returns a custom value of Error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error. 
     */
    Nebulite::Constants::Error func_return(int argc, char* argv[]);

    /**
     * @brief Echoes all arguments as string to the standard output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error echo(int argc, char* argv[]);


    // Assert CRITICAL_CUSTOM_ASSERT
    /**
     * @brief Asserts a condition and throws a custom error if false
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_assert(int argc, char* argv[]);

    /**
     * @brief Attach a command to the always-taskqueue that is executed on each tick.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <command>. The command to attach.
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error always(int argc, char* argv[]);

    /**
     * @brief Clears the entire always-taskqueue.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error alwaysClear(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, General){
        bindFunction(&General::eval,                "eval",                 "Evaluate an expression and execute the result. Example: eval echo $(1+1)");
        bindFunction(&General::exitProgram,         "exit",                 "Exit the program");
        bindFunction(&General::wait,                "wait",                 "Wait a given amount of frames: wait <frames>");
        bindFunction(&General::loadTaskList,        "task",                 "Load a task list from a file: task <filename>");
        bindFunction(&General::forLoop,             "for",                  "Execute a for-loop with a function call: for <var> <start> <end> <functioncall>");
        bindFunction(&General::ifCondition,         "if",                   "Execute a block of code if a condition is true: if <condition> <functioncall>");
        bindFunction(&General::func_return,         "return",               "Return a custom value");
        bindFunction(&General::echo,                "echo",                 "Echo a string to cout: echo <string>");
        bindFunction(&General::func_assert,         "assert",               "Assert a condition and throw an error if false: assert <condition>");
        bindFunction(&General::always,              "always",               "Attach function to always run: always <command>");
        bindFunction(&General::alwaysClear,         "always-clear",         "Clear all always-tasks");
    }
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite