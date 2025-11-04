/**
 * @file GDM_StateManagement.hpp
 * @brief Header file for the StateManagement DomainModule of the GlobalSpace tree.
 */

#pragma once

//------------------------------------------
// Includes

// General
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
 * @class Nebulite::DomainModule::GlobalSpace::StateManagement
 * @brief DomainModule for state management within the GlobalSpace.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, StateManagement) {
public:
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Saves the current game state under state prefix
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     * 
     * @todo not implemented
     */
    Nebulite::Constants::Error stateSave(int argc, char* argv[]);

    /**
     * @brief Loads a saved game state
     * 
     * @param argc The argument count
     * @param argv The argument vector: <name>
     * @return Potential errors that occurred on command execution
     * 
     * @todo not implemented
     */
    Nebulite::Constants::Error stateLoad(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, StateManagement){
        bindSubtree("state", "State management functions");
        bindFunction(&StateManagement::stateSave,       "state save",    "Save the current game state: state-save <name>");
        bindFunction(&StateManagement::stateLoad,       "state load",    "Load a saved game state: state-load <name>");
    }
};
} // namespace GlobalSpace
} // namespace DomainModule
} // namespace Nebulite