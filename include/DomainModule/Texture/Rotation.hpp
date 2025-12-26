/**
 * @file Rotation.hpp
 * @todo Functionality is not given, errors regarding streaming access / target setting persist.
 *       Prioritize later, once some more important features are overhauled.
 */

#ifndef NEBULITE_TXDM_ROTATION_HPP
#define NEBULITE_TXDM_ROTATION_HPP

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
 * @class Nebulite::DomainModule::Texture::Rotation
 * @brief DomainModule for rotation functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, Rotation) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error rotate(int argc, char** argv);
    static std::string_view constexpr rotate_name = "rotate";
    static std::string_view constexpr rotate_desc = "Rotate the texture by a specified angle.\n"
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
} // namespace Nebulite::DomainModule::Texture
#endif // NEBULITE_TXDM_ROTATION_HPP
