/**
 * @file Fill.hpp
 * @todo Functionality is not given, errors regarding streaming access / target setting persist.
 *       Prioritize later, once some more important features are overhauled.
 */

#ifndef NEBULITE_MODULE_DOMAIN_TEXTURE_FILL_HPP
#define NEBULITE_MODULE_DOMAIN_TEXTURE_FILL_HPP

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
 * @class Nebulite::Module::Domain::Texture::Fill
 * @brief DomainModule for fill functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, Fill) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event fill(int argc, char** argv) const ;
    static auto constexpr fill_name = "fill";
    static auto constexpr fill_desc = "Fills the texture with a specified color.\n"
        "Usage:\n"
        "  fill <color>\n"
        "  fill <r> <g> <b>\n"
        "Where <color> can be 'red', 'green', or 'blue',\n"
        "and <r>, <g>, <b> are integer values (0-255) for red, green, and blue components.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, Fill) {
        bindFunction(&Fill::fill, fill_name, fill_desc);
    }
};
} // namespace Nebulite::Module::Domain::Texture
#endif // NEBULITE_MODULE_DOMAIN_TEXTURE_FILL_HPP
