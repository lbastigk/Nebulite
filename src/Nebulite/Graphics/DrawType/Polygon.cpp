//------------------------------------------
// Includes

// Standard library
#include <limits>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Core/Texture.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Graphics/DrawType/Polygon.hpp"
#include "Nebulite/Graphics/DrawcallRefs.hpp"
#include "Nebulite/Graphics/Sdl/Geometry.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Math/Vec2.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DrawType {

Polygon::Polygon(Data::JsonScope& scope, DrawcallRefs const& refs) {
    // Setup destination rect if not already defined
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstX) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstX, 0.0);
    }
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstY) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstY, 0.0);
    }
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstW) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstW, *refs.rectSrcW);
    }
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstH) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstH, *refs.rectSrcH);
    }

    // Get polygon points
    pointCount = scope.memberSize(Key::points);
    points.reserve(pointCount);
    for (auto const key : Key::points.getArrayKeys(pointCount)) {
        auto const pointX = *refs.rectSrcW * scope.get<double>(key.addMember("x")).value_or(0.0);
        auto const pointY = *refs.rectSrcH * scope.get<double>(key.addMember("y")).value_or(0.0);
        points.push_back({ .x=static_cast<float>(pointX), .y=static_cast<float>(pointY) });
    }
}

void Polygon::drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& /*scope*/, DrawcallRefs& refs) {
    SDL_Renderer* sdlRenderer = renderer.getSdlRenderer();
    if (!sdlRenderer) {
        texture.capture.error.println("Renderer not available for circle drawcall.");
        return;
    }
    if (pointCount < 2) { // Bump to 3 later on for filled polygons
        texture.capture.error.println("Polygon drawcall requires at least 2 points.");
        return;
    }

    // Determine polygon size
    double const w = *refs.rectSrcW;
    double const h = *refs.rectSrcH;
    if (w < std::numeric_limits<double>::epsilon() || h < std::numeric_limits<double>::epsilon()) {
        texture.capture.error.println("Polygon drawcall has invalid src rect size. w and h must be > 0.");
        return;
    }

    // Create a texture for the polygon
    SDL_Texture* polyTexture = SDL_CreateTexture(
        sdlRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        static_cast<int>(w),
        static_cast<int>(h)
    );
    polyColor = {
        .r=static_cast<Uint8>(*refs.colorR),
        .g=static_cast<Uint8>(*refs.colorG),
        .b=static_cast<Uint8>(*refs.colorB),
        .a=static_cast<Uint8>(*refs.colorA),
    };

    if (!Math::isZero(*polygonFilled)) {
        // Filled polygon
        Sdl::Geometry::drawFilledPolygon(sdlRenderer, polyTexture, polyColor, points);
    }
    else {
        // Empty polygon
        Sdl::Geometry::drawEmptyPolygon(sdlRenderer, polyTexture, polyColor, points);
    }

    // Check for errors
    if (!polyTexture) {
        texture.capture.error.println("Failed to create polygon texture: ", SDL_GetError());
        return;
    }
    texture.linkExternalTexture(polyTexture);
}

bool Polygon::diff(Data::JsonScope& scope, DrawcallRefs& refs) {
    return pointCount != scope.memberSize(Key::points)
        || polyColor.r != static_cast<Uint8>(*refs.colorR)
        || polyColor.g != static_cast<Uint8>(*refs.colorG)
        || polyColor.b != static_cast<Uint8>(*refs.colorB)
        || polyColor.a != static_cast<Uint8>(*refs.colorA);
}

Math::Vec2<float> Polygon::getRenderOffset(DrawcallRefs& /*refs*/) {
    return Math::Vec2<float>{
        .x=0.0f,
        .y=0.0f
    };
}

} // namespace Nebulite::Graphics::DrawType
