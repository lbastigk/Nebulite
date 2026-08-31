//------------------------------------------
// Includes

// Standard library
#include <utility>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Core/Texture.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/DrawType/Text.hpp"
#include "Nebulite/Graphics/DrawcallRefs.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DrawType {

void Text::setStandardTextRectsIfMissing(Data::JsonScope& scope, float const w, float const h, TTF_Font* font) const {
    // Cast to double
    double const srcW = static_cast<double>(w) * 1.0;
    double const srcH = static_cast<double>(h) * 1.0;
    double const dstW = srcW * *textFontsize / static_cast<double>(TTF_GetFontSize(font));
    double const dstH = srcH * *textFontsize / static_cast<double>(TTF_GetFontSize(font));

    // Setup src values unless they are already defined
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcX) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcX, 0.0);
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcY) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcY, 0.0);
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcW) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcW, srcW);
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcH) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcH, srcH);

    // Prefer measured pixel size for dst unless the caller explicitly set different values
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstX) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstX, 0.0);
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstY) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstY, 0.0);
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstW) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstW, dstW);
    if (scope.memberType(Constants::KeyNames::Drawcall::Rect::dstH) != Data::KeyType::value) scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstH, dstH);
}

Text::Text(Data::JsonScope& scope, DrawcallRefs& refs) : textFontsize(scope.getStableDoublePointer(Key::fontsize)) {
    text = scope.get<std::string>(Key::str).value_or("");
    if (text.empty()) {
        text = " "; // Render at least a space to get height
    }
    textColor = {
        .r=static_cast<Uint8>(*refs.colorR),
        .g=static_cast<Uint8>(*refs.colorG),
        .b=static_cast<Uint8>(*refs.colorB),
        .a=static_cast<Uint8>(*refs.colorA),
    };
}

void Text::drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& /*refs*/) {
    SDL_Renderer* sdl = renderer.getSdlRenderer();
    if (!sdl) {
        texture.capture.error.println("Renderer not available for text drawcall.");
        return;
    }

    TTF_Font* font = renderer.getStandardFont();
    if (!font) {
        texture.capture.error.println("Font not available for text drawcall.");
        return;
    }

    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, textColor, 0);
    if (!surf) {
        texture.capture.error.println("TTF_RenderText_Blended_Wrapped failed: ", SDL_GetError());
        return;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl, surf);
    SDL_DestroySurface(surf);
    if (!tex) {
        texture.capture.error.println("SDL_CreateTextureFromSurface failed: ", SDL_GetError());
        return;
    }

    float w = 0;
    float h = 0;
    if (!SDL_GetTextureSize(tex, &w, &h)) {
        texture.capture.error.println("SDL_GetTextureSize failed: ", SDL_GetError());
        SDL_DestroyTexture(tex);
        return;
    }

    setStandardTextRectsIfMissing(scope, w, h, font);
    texture.setInternalTexture(tex);
}

bool Text::diff(Data::JsonScope& scope, DrawcallRefs& refs) {
    return text != scope.get<std::string>(Key::str).value_or("")
        || textColor.r != static_cast<Uint8>(*refs.colorR)
        || textColor.g != static_cast<Uint8>(*refs.colorG)
        || textColor.b != static_cast<Uint8>(*refs.colorB)
        || textColor.a != static_cast<Uint8>(*refs.colorA);
}

std::pair<float,float> Text::getRenderOffset(DrawcallRefs& /*refs*/) {
    return std::make_pair(0.0, 0.0);
}

} // namespace Nebulite::Graphics::DrawType
