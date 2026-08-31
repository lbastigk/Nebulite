#ifndef NEBULITE_GRAPHICS_DRAWTYPE_SPRITE_HPP
#define NEBULITE_GRAPHICS_DRAWTYPE_SPRITE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <utility>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Graphics/DrawType/DrawType.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
class Texture;
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Graphics {
struct DrawcallRefs;
} // namespace Nebulite::Graphics

//------------------------------------------
namespace Nebulite::Graphics::DrawType {
/**
 * @brief Specialization for drawing Sprites
 */
class Sprite : public DrawType {
    std::string link;

public:
    struct Key {
        static auto constexpr imageLocation = Data::ScopedKeyView("textureData.link");
    };

    Sprite(Data::JsonScope& scope, DrawcallRefs& /*refs*/);

    void drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& /*refs*/) override ;

    bool diff(Data::JsonScope& scope, DrawcallRefs& refs) override ;

    std::pair<float,float> getRenderOffset(DrawcallRefs& /*refs*/) override ;
};
} // namespace Nebulite::Graphics::DrawType
#endif // NEBULITE_GRAPHICS_DRAWTYPE_SPRITE_HPP