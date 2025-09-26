/**
 * @file TDM_General.hpp
 * 
 * @brief This file contains the DomainModule of the Texture for general functions.
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
 * @class Nebulite::DomainModule::Texture::General
 * @brief DomainModule for general functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, General) {
public:
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Reloads the texture from the document.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments required, texture is reloaded from document
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error reloadTexture(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, General) {
        bindFunction(&General::reloadTexture, "reload-texture",   "Reload the texture from the document.");
    }
};
}   // namespace Texture
}   // namespace DomainModule
}   // namespace Nebulite