/**
 * @file General.hpp
 * @brief This file contains the DomainModule of the Texture for general functions.
 */

#ifndef NEBULITE_DOMAINMODULE_TEXTURE_GENERAL_HPP
#define NEBULITE_DOMAINMODULE_TEXTURE_GENERAL_HPP

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
    [[nodiscard]] Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Error reloadTexture() const;
    static auto constexpr reloadTexture_name = "reload-texture";
    static auto constexpr reloadTexture_desc = "Reload the texture from the document.\n"
        "\n"
        "Usage: reload-texture\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, General) {
        BIND_FUNCTION(&General::reloadTexture, reloadTexture_name, reloadTexture_desc);
    }
};
} // namespace Nebulite::DomainModule::Texture
#endif // NEBULITE_DOMAINMODULE_TEXTURE_GENERAL_HPP
