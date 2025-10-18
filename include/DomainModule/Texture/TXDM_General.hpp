/**
 * @file TXDM_General.hpp
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
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Reloads the texture from the document.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments required, texture is reloaded from document
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error reloadTexture([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]);
    static const std::string reloadTexture_name;
    static const std::string reloadTexture_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, General) {
        bindFunction(&General::reloadTexture, reloadTexture_name, &reloadTexture_desc);
    }
};
}   // namespace Texture
}   // namespace DomainModule
}   // namespace Nebulite