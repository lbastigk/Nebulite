/**
 * @file RDM_Debug.hpp
 * 
 * Debug functions for the the domain RenderObject.
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
        class RenderObject; // Forward declaration of domain class RenderObject
    }
}

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace RenderObject{
/**
 * @class Nebulite::DomainModule::RenderObject::Debug
 * @brief Debug management for the RenderObject tree DomainModule.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Debug) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * Same as for GlobalSpace within GDM_General.hpp, but local to the RenderObject
     * for variable resolution.
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
     * @brief Prints the source rectangle of the spritesheet to console
     * 
     * @param argc The argument count
     * @param argv The argument vector: None
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error printSrcRect(int argc, char* argv[]);

    /**
     * @brief Prints the destination rectangle of the spritesheet to console
     * 
     * @param argc The argument count
     * @param argv The argument vector: None
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::Error printDstRect(int argc, char* argv[]);

    /**
     * @brief Prints the texture status to cout
     */
    Nebulite::Constants::Error textureStatus(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Debug) {
        // Some functions like selected-object need eval to resolve variables
        bindFunction(&Debug::eval,          "eval",                     "Evaluate an expression and execute the result. Example: eval echo $(1+1)");

        bindSubtree("debug", "Debugging functions for RenderObject");
        bindFunction(&Debug::printSrcRect,  "debug print-src-rect",     "Prints the source rectangle of the spritesheet to console");
        bindFunction(&Debug::printDstRect,  "debug print-dst-rect",     "Prints the destination rectangle of the spritesheet to console");
        bindFunction(&Debug::textureStatus, "debug texture-status",     "Prints texture status to the console");


    }
};
}   // namespace DomainModule
}   // namespace RenderObject
}   // namespace Nebulite