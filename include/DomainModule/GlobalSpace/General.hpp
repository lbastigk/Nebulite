/**
 * @file General.hpp
 * @brief General-purpose DomainModule for the GlobalSpace.
 */

#ifndef NEBULITE_GSDM_GENERAL_HPP
#define NEBULITE_GSDM_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of domain class GlobalSpace
}

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::General
 * @brief DomainModule for general-purpose functions within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, General) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions
    // See _desc for documentation

    Constants::Error eval(int argc, char** argv);
    static std::string const eval_name;
    static std::string const eval_desc;

    Constants::Error exit(int argc, char** argv);
    static std::string const exit_name;
    static std::string const exit_desc;

    Constants::Error wait(int argc, char** argv);
    static std::string const wait_name;
    static std::string const wait_desc;

    Constants::Error task(int argc, char** argv);
    static std::string const task_name;
    static std::string const task_desc;

    Constants::Error func_for(int argc, char** argv);
    static std::string const func_for_name;
    static std::string const func_for_desc;

    Constants::Error func_if(int argc, char** argv);
    static std::string const func_if_name;
    static std::string const func_if_desc;

    Constants::Error func_return(int argc, char** argv);
    static std::string const func_return_name;
    static std::string const func_return_desc;

    Constants::Error echo(int argc, char** argv);
    static std::string const echo_name;
    static std::string const echo_desc;

    Constants::Error func_assert(int argc, char** argv);
    static std::string const assert_name;
    static std::string const assert_desc;

    Constants::Error always(int argc, char** argv);
    static std::string const always_name;
    static std::string const always_desc;

    Constants::Error alwaysClear(int argc, char** argv);
    static std::string const alwaysClear_name;
    static std::string const alwaysClear_desc;

    Constants::Error nop(std::span<std::string const> const& args);
    static std::string const nop_name;
    static std::string const nop_desc;

    //------------------------------------------
    // Category names

    // None, general functions should be in the root of the GlobalSpace
    // Otherwise they should not be considered general-purpose

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, General) {
        bindFunction(&General::eval, eval_name, eval_desc);
        bindFunction(&General::exit, exit_name, exit_desc);
        bindFunction(&General::wait, wait_name, wait_desc);
        bindFunction(&General::task, task_name, task_desc);
        bindFunction(&General::func_for, func_for_name, func_for_desc);
        bindFunction(&General::func_if, func_if_name, func_if_desc);
        bindFunction(&General::func_return, func_return_name, func_return_desc);
        bindFunction(&General::echo, echo_name, echo_desc);
        bindFunction(&General::func_assert, assert_name, assert_desc);
        bindFunction(&General::always, always_name, always_desc);
        bindFunction(&General::alwaysClear, alwaysClear_name, alwaysClear_desc);
        bindFunction(&General::nop, nop_name, nop_desc);
    }
};
} // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_GSDM_GENERAL_HPP
