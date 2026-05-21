/**
 * @file General.hpp
 * @brief This file contains the DomainModule of the Texture for general functions.
 */

#ifndef MODULE_DOMAIN_TEXTURE_GENERAL_HPP
#define MODULE_DOMAIN_TEXTURE_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Texture;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Texture {
/**
 * @class Nebulite::Module::Domain::Texture::General
 * @brief DomainModule for general functions within the Texture.
 */
class General final : public Base::DomainModule<Core::Texture> {
public:
    [[nodiscard]] Constants::Event updateHook() override ;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event reloadTexture() const;
    static auto constexpr reloadTexture_name = "reload-texture";
    static auto constexpr reloadTexture_desc = "Reload the texture from the document.\n"
        "\n"
        "Usage: reload-texture\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit General(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&General::reloadTexture, reloadTexture_name, reloadTexture_desc);
    }
};
} // namespace Nebulite::Module::Domain::Texture
#endif // MODULE_DOMAIN_TEXTURE_GENERAL_HPP
