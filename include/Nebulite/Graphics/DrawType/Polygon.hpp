#ifndef NEBULITE_GRAPHICS_DRAWTYPE_POLYGON_HPP
#define NEBULITE_GRAPHICS_DRAWTYPE_POLYGON_HPP

//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <utility>
#include <vector>

// External
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

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
 * @brief Specialization for drawing Polygons
 */
class Polygon : public DrawType {
    std::vector<SDL_FPoint> points;
    std::size_t pointCount{};
    SDL_Color polyColor{.r=0,.g=0,.b=0,.a=0};
    double* polygonFilled = nullptr;

public:
    struct Key {
        static auto constexpr points = Data::ScopedKeyView("textureData.points"); // Array of point objects {x, y}
        static auto constexpr filled = Data::ScopedKeyView("textureData.filled"); // bool
    };

    Polygon(Data::JsonScope& scope, DrawcallRefs& refs);

    void drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& refs) override ;

    bool diff(Data::JsonScope& scope, DrawcallRefs& refs) override ;

    std::pair<float,float> getRenderOffset(DrawcallRefs& /*refs*/) override ;
};
} // namespace Nebulite::Graphics::DrawType
#endif // NEBULITE_GRAPHICS_DRAWTYPE_POLYGON_HPP
