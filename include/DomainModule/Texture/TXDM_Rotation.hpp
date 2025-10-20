/**
 * @file TXDM_Rotation.hpp
 * 
 * This file contains the DomainModule of the Texture for rotation functions.
 * 
 * @todo Functionality is not given, errors regarding streaming access / target setting persist.
 * Prioritize later, once some more important features are overhauled.
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
        class Texture; // Forward declaration of domain class Texture
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace Texture {
/**
 * @class Nebulite::DomainModule::Texture::Rotation
 * @brief DomainModule for rotation functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, Rotation) {
public:
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Evaluates an expression string and executes it
     * 
     * @param argc The argument count
     * @param argv The argument vector: rotation angle in degrees
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error rotate(int argc,  char* argv[]);
    static const std::string rotate_name;
    static const std::string rotate_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, Rotation) {
        bindFunction(&Rotation::rotate, rotate_name, &rotate_desc);
    }
};
}   // namespace Texture
}   // namespace DomainModule
}   // namespace Nebulite