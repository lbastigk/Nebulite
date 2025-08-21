/**
 * @file GTE_General.h
 * 
 * This file contains an expansion of the GlobalSpaceTree for general-purpose functions.
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace


namespace GlobalSpaceTreeExpansion {

/**
 * @class Nebulite::GlobalSpaceTreeExpansion::General
 * @brief General-purpose functions for the GlobalSpaceTree.
 * 
 * This class binds general-purpose functions to the GlobalSpaceTree.
 */
class General : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, General> {
public:
    using Wrapper<Nebulite::GlobalSpace, General>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * @param argc The argument count
     * @param argv The argument vector: the string to evaluate
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * Examples:
     * 
     * eval echo $(1+1)    outputs:    2.000000
     * eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
     */
    Nebulite::ERROR_TYPE eval(int argc, char* argv[]);

    /**
     * @brief Exits the entire program
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo In order to properly exit, it we must make sure to clean up the taskQueue:
     *  ./bin/Nebulite "echo 1; exit ; echo 2"
     * Will still echo 2
     */
    Nebulite::ERROR_TYPE exitProgram(int argc, char* argv[]);

    /**
     * @brief Sets the waitCounter to the given value to halt all script tasks for a given amount of frames
     * 
     * @param argc The argument count
     * @param argv The argument vector: frame count to wait
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE wait(int argc, char* argv[]);

    /**
     * @brief Loads a task list from a file
     * 
     * @param argc The argument count
     * @param argv The argument vector: the filename to load
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE loadTaskList(int argc, char* argv[]);

    /**
     * @brief Executes a for-loop with a function call
     * 
     * @param argc The argument count
     * @param argv The argument vector: <var> <start> <end> <functioncall>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo Modify for-variables to be conform to standard variables:
     *       Currently: for i 0 10 echo $i
     *       New:       for i 0 10 echo {i}
     */
    Nebulite::ERROR_TYPE forLoop(int argc, char* argv[]);

    /**
     * @brief Executes a block of code if a condition is true
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition> <functioncall>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE ifCondition(int argc, char* argv[]);

    /**
     * @brief Returns a custom value of ERROR_TYPE
     * 
     * @param argc The argument count
     * @param argv The argument vector: <value>
     * @return The specified value of ERROR_TYPE. 
     * Returns Nebulite::ERROR_TYPE::TOO_FEW_ARGS if no value is provided
     * or Nebulite::ERROR_TYPE::TOO_MANY_ARGS if too many values are provided
     */
    Nebulite::ERROR_TYPE func_return(int argc, char* argv[]);

    // Echo a given string to cout
    /**
     * @brief Echoes all arguments as string to the standard output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE echo(int argc, char* argv[]);

    /**
     * @brief Echoes all arguments as string to the standard error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo Move to GTE_Debug
     */
    Nebulite::ERROR_TYPE error(int argc, char* argv[]);

    // Assert CRITICAL_CUSTOM_ASSERT
    /**
     * @brief Asserts a condition and throws a custom error if false
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     */
    Nebulite::ERROR_TYPE func_assert(int argc, char* argv[]);

    // Force a global value to a certain value
    /**
     * @brief Forces a global variable to a specific value
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo move to GTE_InputMapping
     */
    Nebulite::ERROR_TYPE forceGlobal(int argc, char* argv[]);

    // Release all forced global values
    /**
     * @brief Clears all forced global variables
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo move to GTE_InputMapping
     */
    Nebulite::ERROR_TYPE clearForceGlobal(int argc, char* argv[]);

    // Save entire game state
    /**
     * @brief Saves the current game state under state prefix
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo not implemented, move to separate expansion GTE_StateManagement
     */
    Nebulite::ERROR_TYPE stateSave(int argc, char* argv[]);

    // Load game state
    /**
     * @brief Loads a saved game state
     * 
     * @param argc The argument count
     * @param argv The argument vector: <name>
     * @return Nebulite::ERROR_TYPE Potential errors that occured on command execution
     * 
     * @todo not implemented, move to separate expansion GTE_StateManagement
     */
    Nebulite::ERROR_TYPE stateLoad(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&General::eval,                "eval",                 "Evaluate an expression and execute the result. Example: eval echo $(1+1)");
        bindFunction(&General::exitProgram,         "exit",                 "Exit the program");
        bindFunction(&General::wait,                "wait",                 "Wait a given amount of frames: wait <frames>");
        bindFunction(&General::loadTaskList,        "task",                 "Load a task list from a file: task <filename>");
        bindFunction(&General::forLoop,             "for",                  "Execute a for-loop with a function call: for <var> <start> <end> <functioncall>");
        bindFunction(&General::ifCondition,         "if",                   "Execute a block of code if a condition is true: if <condition> <functioncall>");
        bindFunction(&General::func_return,         "return",               "Return a custom value");
        bindFunction(&General::echo,                "echo",                 "Echo a string to cout: echo <string>");
        bindFunction(&General::error,               "error",                "Echo a string to cerr/errorfile: error <string>");
        bindFunction(&General::func_assert,         "assert",               "Assert a condition and throw an error if false: assert <condition>");
        bindFunction(&General::forceGlobal,         "force-global",         "Force a global variable to a value: force-global <key> <value>");
        bindFunction(&General::clearForceGlobal,    "force-global-clear",   "Clear all forced global variables");
        bindFunction(&General::stateSave,           "state-save",           "Save the current game state: state-save <name>");
        bindFunction(&General::stateLoad,           "state-load",           "Load a saved game state: state-load <name>");
    }
};
}
}