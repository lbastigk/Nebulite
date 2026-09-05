//------------------------------------------
// Includes

// External
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Core/Texture.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/DrawType/Circle.hpp"
#include "Nebulite/Graphics/DrawcallRefs.hpp"
#include "Nebulite/Graphics/Sdl/Geometry.hpp"
#include "Nebulite/Math/Vec2.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DrawType {

Circle::Circle(Data::JsonScope& scope, DrawcallRefs const& refs) : circleRadius(scope.getStableDoublePointer(Key::radius)) {
    circleColor = {
        .r=static_cast<Uint8>(*refs.colorR),
        .g=static_cast<Uint8>(*refs.colorG),
        .b=static_cast<Uint8>(*refs.colorB),
        .a=static_cast<Uint8>(*refs.colorA),
    };

    // Setup src values unless they are already defined
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcX) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcX, 0.0);
    }
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcY) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcY, 0.0);
    }
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcW) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcW, 2*static_cast<int>(*circleRadius));
    }
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcH) != Data::KeyType::value) {
        scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcH, 2*static_cast<int>(*circleRadius));
    }
}

void Circle::drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& /*scope*/, DrawcallRefs& /*refs*/) {
    SDL_Renderer* sdlRenderer = renderer.getSdlRenderer();
    if (!sdlRenderer) {
        texture.capture.error.println("Renderer not available for circle drawcall.");
        return;
    }
    SDL_Texture* circleTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, static_cast<int>(*circleRadius) * 2, static_cast<int>(*circleRadius) * 2);
    SDL_SetRenderDrawColor(sdlRenderer, circleColor.r, circleColor.g, circleColor.b, circleColor.a);
    Sdl::Geometry::drawFilledCircle(sdlRenderer, circleTexture, circleColor, static_cast<int>(*circleRadius));
    texture.setInternalTexture(circleTexture);
}

bool Circle::diff(Data::JsonScope& /*scope*/, DrawcallRefs& refs) {
    return radius != static_cast<int>(*circleRadius)
        || circleColor.r != static_cast<Uint8>(*refs.colorR)
        || circleColor.g != static_cast<Uint8>(*refs.colorG)
        || circleColor.b != static_cast<Uint8>(*refs.colorB)
        || circleColor.a != static_cast<Uint8>(*refs.colorA);
}
Math::Vec2<float> Circle::getRenderOffset(DrawcallRefs& refs) {
    return Math::Vec2<float>{
        .x = static_cast<float>(*refs.rectDstW / 2.0),
        .y = static_cast<float>(*refs.rectDstH / 2.0),
    };
}

} // namespace Nebulite::Graphics::DrawType
