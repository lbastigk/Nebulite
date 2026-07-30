#ifndef NEBULITE_MODULE_DOMAIN_TEXTURE_GENERAL_HPP
#define NEBULITE_MODULE_DOMAIN_TEXTURE_GENERAL_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

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
    static auto constexpr reloadTextureName = "reload-texture";
    static auto constexpr reloadTextureDesc = "Reload the texture from the document.\n"
        "\n"
        "Usage: reload-texture\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit General(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&General::reloadTexture, reloadTextureName, reloadTextureDesc);
    }
};
} // namespace Nebulite::Module::Domain::Texture
#endif // NEBULITE_MODULE_DOMAIN_TEXTURE_GENERAL_HPP
