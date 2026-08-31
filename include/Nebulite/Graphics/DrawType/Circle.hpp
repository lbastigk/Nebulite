#ifndef NEBULITE_GRAPHICS_DRAWTYPE_CIRCLE_HPP
#define NEBULITE_GRAPHICS_DRAWTYPE_CIRCLE_HPP

//------------------------------------------
// Includes

// Standard library
#include <utility>

// External
#include <SDL3/SDL_pixels.h>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Graphics/DrawType/DrawType.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
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
 * @brief Specialization for drawing Circles
 */
class Circle : public DrawType {
    int radius{};
    SDL_Color circleColor{.r=0,.g=0,.b=0,.a=0};
    double* circleRadius = nullptr;

public:
    struct Key {
        static auto constexpr radius = Data::ScopedKeyView("textureData.radius");
    };

    Circle(Data::JsonScope& scope, DrawcallRefs& refs);

    void drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& refs) override ;

    bool diff(Data::JsonScope& /*scope*/, DrawcallRefs& refs) override ;

    std::pair<float,float> getRenderOffset(DrawcallRefs& refs) override ;
};
} // namespace Nebulite::Graphics::DrawType
#endif // NEBULITE_GRAPHICS_DRAWTYPE_CIRCLE_HPP