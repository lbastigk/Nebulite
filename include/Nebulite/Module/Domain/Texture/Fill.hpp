#ifndef NEBULITE_MODULE_DOMAIN_TEXTURE_FILL_HPP
#define NEBULITE_MODULE_DOMAIN_TEXTURE_FILL_HPP

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
 * @class Nebulite::Module::Domain::Texture::Fill
 * @brief DomainModule for fill functions within the Texture.
 */
class Fill final : public Base::DomainModule<Core::Texture> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] Constants::Event fill(int argc, char const** argv) const ;
    static auto constexpr fill_name = "fill";
    static auto constexpr fill_desc = "Fills the texture with a specified color.\n"
        "Usage:\n"
        "  fill <color>\n"
        "  fill <r> <g> <b>\n"
        "Where <color> can be 'red', 'green', or 'blue',\n"
        "and <r>, <g>, <b> are integer values (0-255) for red, green, and blue components.\n";

    [[nodiscard]] Constants::Event averageColor() const;
    static auto constexpr averageColor_name = "average-color";
    static auto constexpr averageColor_desc = "Prints the average color of the texture.\n"
        "Format: Average color of texture: R=<value> G=<value> B=<value> A=<value>\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Fill(ConstructorParams const& params) : DomainModule(params) {
        bindFunction(&Fill::fill, fill_name, fill_desc);
        bindFunction(&Fill::averageColor, averageColor_name, averageColor_desc);
    }
};
} // namespace Nebulite::Module::Domain::Texture
#endif // NEBULITE_MODULE_DOMAIN_TEXTURE_FILL_HPP
