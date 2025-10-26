/**
 * @file GSDM_General.hpp
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
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, General){
public:
    Nebulite::Constants::Error update() override;

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
    Nebulite::Constants::Error eval(int argc,  char* argv[]);
    static std::string const eval_name;
    static std::string const eval_desc;

    /**
     * @brief Exits the entire program
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error exit(int argc,  char* argv[]);
    static std::string const exit_name;
    static std::string const exit_desc;

    /**
     * @brief Sets the waitCounter to the given value to halt all script tasks for a given amount of frames
     * 
     * @param argc The argument count
     * @param argv The argument vector: frame count to wait
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error wait(int argc,  char* argv[]);
    static std::string const wait_name;
    static std::string const wait_desc;

    /**
     * @brief Loads tasks from a file into the taskQueue
     * 
     * @param argc The argument count
     * @param argv The argument vector: the filename to load
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error task(int argc,  char* argv[]);
    static std::string const task_name;
    static std::string const task_desc;

    /**
     * @brief Executes a for-loop with a function call
     * 
     * @param argc The argument count
     * @param argv The argument vector: <var> <start> <end> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_for(int argc,  char* argv[]);
    static std::string const func_for_name;
    static std::string const func_for_desc;

    /**
     * @brief Executes a block of code if a condition is true
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition> <functioncall>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_if(int argc,  char* argv[]);
    static std::string const func_if_name;
    static std::string const func_if_desc;

    /**
     * @brief Returns a custom value of type Critical Error
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return The specified value of Error. 
     */
    Nebulite::Constants::Error func_return(int argc,  char* argv[]);
    static std::string const func_return_name;
    static std::string const func_return_desc;

    /**
     * @brief Echoes all arguments as string to the standard output
     * 
     * @param argc The argument count
     * @param argv The argument vector: <string>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error echo(int argc,  char* argv[]);
    static std::string const echo_name;
    static std::string const echo_desc;

    /**
     * @brief Asserts a condition and throws a custom error if false
     * 
     * @param argc The argument count
     * @param argv The argument vector: <condition>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error func_assert(int argc,  char* argv[]);
    static std::string const assert_name;
    static std::string const assert_desc;

    /**
     * @brief Attach a command to the always-taskqueue that is executed on each tick.
     * 
     * @param argc The argument count
     * @param argv The argument vector: inputs are <command>. The command to attach.
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error always(int argc,  char* argv[]);
    static std::string const always_name;
    static std::string const always_desc;

    /**
     * @brief Clears the entire always-taskqueue.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error alwaysClear(int argc,  char* argv[]);
    static std::string const alwaysClear_name;
    static std::string const alwaysClear_desc;

    //------------------------------------------
    // Category names


    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, General){
        bindFunction(&General::eval,        eval_name,          &eval_desc);
        bindFunction(&General::exit,        exit_name,          &exit_desc);
        bindFunction(&General::wait,        wait_name,          &wait_desc);
        bindFunction(&General::task,        task_name,          &task_desc);
        bindFunction(&General::func_for,    func_for_name,      &func_for_desc);
        bindFunction(&General::func_if,     func_if_name,       &func_if_desc);
        bindFunction(&General::func_return, func_return_name,   &func_return_desc);
        bindFunction(&General::echo,        echo_name,          &echo_desc);
        bindFunction(&General::func_assert, assert_name,        &assert_desc);
        bindFunction(&General::always,      always_name,        &always_desc);
        bindFunction(&General::alwaysClear, alwaysClear_name,   &alwaysClear_desc);
    }
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite