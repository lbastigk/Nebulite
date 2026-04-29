/**
 * @file Rotation.hpp
 * @todo Functionality is not given, errors regarding streaming access / target setting persist.
 *       Prioritize later, once some more important features are overhauled.
 */

#ifndef NEBULITE_MODULE_DOMAIN_TEXTURE_ROTATION_HPP
#define NEBULITE_MODULE_DOMAIN_TEXTURE_ROTATION_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Texture;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Texture {
/**
 * @class Nebulite::Module::Domain::Texture::Rotation
 * @brief DomainModule for rotation functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, Rotation) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event rotate(int argc, char** argv) const ;
    static auto constexpr rotate_name = "rotate";
    static auto constexpr rotate_desc = "Rotate the texture by a specified angle.\n"
        "\n"
        "Usage: rotate <angle>\n"
        "\n"
        "Arguments:\n"
        "  angle   The angle in degrees to rotate the texture.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, Rotation) {
        bindFunction(&Rotation::rotate, rotate_name, rotate_desc);
    }
};
} // namespace Nebulite::Module::Domain::Texture
#endif // NEBULITE_MODULE_DOMAIN_TEXTURE_ROTATION_HPP
