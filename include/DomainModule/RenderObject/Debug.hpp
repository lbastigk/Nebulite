/**
 * @file Debug.hpp
 * @brief Debug functions for the domain RenderObject.
 */

#ifndef NEBULITE_RODM_DEBUG_HPP
#define NEBULITE_RODM_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
}

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {
/**
 * @class Nebulite::DomainModule::RenderObject::Debug
 * @brief Debug management for the RenderObject tree DomainModule.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::RenderObject, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * Same as for GlobalSpace within GSDM_General.hpp, but local to the RenderObject
     * for variable resolution.
     * 
     * Examples:
     * 
     * eval echo $(1+1)    outputs:    2.000000
     * eval spawn ./Resources/RenderObjects/{global.ToSpawn}.json
     * 
     * @param argc The argument count
     * @param argv The argument vector: the string to evaluate
     * @return Potential errors that occurred on command execution
     */
    Constants::Error eval(int argc, char** argv);
    static std::string const eval_name;
    static std::string const eval_desc;

    /**
     * @brief Prints the source rectangle of the spritesheet to console
     * 
     * @param argc The argument count
     * @param argv The argument vector: None
     * @return Potential errors that occurred on command execution
     */
    Constants::Error printSrcRect(int argc, char** argv);
    static std::string const printSrcRect_name;
    static std::string const printSrcRect_desc;

    /**
     * @brief Prints the destination rectangle of the spritesheet to console
     * 
     * @param argc The argument count
     * @param argv The argument vector: None
     * @return Potential errors that occurred on command execution
     */
    Constants::Error printDstRect(int argc, char** argv);
    static std::string const printDstRect_name;
    static std::string const printDstRect_desc;

    /**
     * @brief Prints the texture status to cout
     */
    Constants::Error textureStatus(int argc, char** argv);
    static std::string const textureStatus_name;
    static std::string const textureStatus_desc;

    //------------------------------------------
    // Category names
    static std::string const debug_name;
    static std::string const debug_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::RenderObject, Debug) {
        // Some functions like selected-object need eval to resolve variables
        bindFunction(&Debug::eval, eval_name, &eval_desc);

        (void)bindCategory(debug_name, &debug_desc);
        bindFunction(&Debug::printSrcRect, printSrcRect_name, &printSrcRect_desc);
        bindFunction(&Debug::printDstRect, printDstRect_name, &printDstRect_desc);
        bindFunction(&Debug::textureStatus, textureStatus_name, &textureStatus_desc);
    }
};
} // namespace Nebulite::DomainModule::RenderObject
#endif // NEBULITE_RODM_DEBUG_HPP