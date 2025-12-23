/**
 * @file General.hpp
 * @brief This file contains the DomainModule of the Texture for general functions.
 */

#ifndef NEBULITE_TXDM_GENERAL_HPP
#define NEBULITE_TXDM_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Texture;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::Texture {
/**
 * @class Nebulite::DomainModule::Texture::General
 * @brief DomainModule for general functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, General) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Reloads the texture from the document.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments required, texture is reloaded from document
     * @return Potential errors that occurred on command execution
     */
    Constants::Error reloadTexture(int argc, char** argv);
    static std::string const reloadTexture_name;
    static std::string const reloadTexture_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, General) {
        bindFunction(&General::reloadTexture, reloadTexture_name, &reloadTexture_desc);
    }
};
} // namespace Nebulite::DomainModule::Texture
#endif // NEBULITE_TXDM_GENERAL_HPP
