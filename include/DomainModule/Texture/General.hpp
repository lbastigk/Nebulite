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

    Constants::Error reloadTexture();
    static std::string_view constexpr reloadTexture_name = "reload-texture";
    static std::string_view constexpr reloadTexture_desc = "Reload the texture from the document.\n"
        "\n"
        "Usage: reload-texture\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, General) {
        bindFunction(&General::reloadTexture, reloadTexture_name, reloadTexture_desc);
    }
};
} // namespace Nebulite::DomainModule::Texture
#endif // NEBULITE_TXDM_GENERAL_HPP
