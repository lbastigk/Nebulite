/**
 * @file Fill.hpp
 * @todo Functionality is not given, errors regarding streaming access / target setting persist.
 *       Prioritize later, once some more important features are overhauled.
 */

#ifndef NEBULITE_TXDM_FILL_HPP
#define NEBULITE_TXDM_FILL_HPP

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
 * @class Nebulite::DomainModule::Texture::Fill
 * @brief DomainModule for fill functions within the Texture.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Texture, Fill) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions
    
    Constants::Error fill(int argc, char** argv);
    static std::string_view constexpr fill_name = "fill";
    static std::string_view constexpr fill_desc = "Fills the texture with a specified color.\n"
        "Usage:\n"
        "  fill <color>\n"
        "  fill <r> <g> <b>\n"
        "Where <color> can be 'red', 'green', or 'blue',\n"
        "and <r>, <g>, <b> are integer values (0-255) for red, green, and blue components.";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Texture, Fill) {
        bindFunction(&Fill::fill, fill_name, fill_desc);
    }
};
} // namespace Nebulite::DomainModule::Texture
#endif // NEBULITE_TXDM_FILL_HPP
