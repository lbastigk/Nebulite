/**
 * @file GDM_General.hpp
 * 
 * This file contains the DomainModule of the GlobalSpaceTree for general-purpose functions.
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
class General : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
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
    Nebulite::Constants::ERROR_TYPE eval(int argc, char* argv[]);

    /**
     * @brief Exits the entire program
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     * 
     * @todo In order to properly exit, it we must make sure to clean up the taskQueue:
     *  ./bin/Nebulite "echo 1; exit ; echo 2"
     * Will still echo 2
     */
    Nebulite::Constants::ERROR_TYPE exitProgram(int argc, char* argv[]);

    /**
     * @brief Sets the waitCounter to the given value to halt all script tasks for a given amount of frames
     * 
     * @param argc The argument count
     * @param argv The argument vector: frame count to wait
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE wait(int argc, char* argv[]);

    /**
     * @brief Loads a task list from a file
     * 
     * @param argc The argument count
     * @param argv The argument vector: the filename to load
     * @return Potential errors that occured on command execution
     * 
     * @todo: Doesnt remove lines that are purely "#"
     */
    Nebulite::Constants::ERROR_TYPE loadTaskList(int argc, char* argv[]);

    /**
     * @brief Executes a for-loop with a function call
     * 
     * @param argc The argument count
     * @param argv The argument vector: <var> <start> <end> <functioncall>
     * @return Potential errors that occured on command execution
     * 
     * @todo Modify for-variables to be conform to standard variables:
     *       Currently: for i 0 10 echo $i
     *       New:       for i 0 10 echo {i}
     */
    Nebulite::Constants::ERROR_TYPE forLoop(int argc, char* argv[]);

    /**
     * @brief Executes a block of code if a condition is true
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE ifCondition(int argc, char* argv[]);

    /**
     * @brief Returns a custom value of ERROR_TYPE
     * 
     * @param argc The argument count
     * @param argv The argument vector: <value>
     * @return The specified value of ERROR_TYPE. 
     * Returns Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS if no value is provided
     * or Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS if too many values are provided
     */
    Nebulite::Constants::ERROR_TYPE func_return(int argc, char* argv[]);

    /**
     * @brief Echoes all arguments as string to the standard output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE echo(int argc, char* argv[]);

    /**
     * @brief Echoes all arguments as string to the standard error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     * 
     * @todo Move to GDM_Debug
     */
    Nebulite::Constants::ERROR_TYPE error(int argc, char* argv[]);

    // Assert CRITICAL_CUSTOM_ASSERT
    /**
     * @brief Asserts a condition and throws a custom error if false
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE func_assert(int argc, char* argv[]);

    /**
     * @brief Saves the current game state under state prefix
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     * 
     * @todo not implemented, move to separate DomainModule GDM_StateManagement
     */
    Nebulite::Constants::ERROR_TYPE stateSave(int argc, char* argv[]);

    /**
     * @brief Loads a saved game state
     * 
     * @param argc The argument count
     * @param argv The argument vector: <name>
     * @return Potential errors that occured on command execution
     * 
     * @todo not implemented, move to separate DomainModule GDM_StateManagement
     */
    Nebulite::Constants::ERROR_TYPE stateLoad(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    General(std::string moduleName, Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
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

        bindSubtree("state", "State management functions");
        bindFunction(&General::stateSave,           "state save",           "Save the current game state: state-save <name>");
        bindFunction(&General::stateLoad,           "state load",           "Load a saved game state: state-load <name>");
    }
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite