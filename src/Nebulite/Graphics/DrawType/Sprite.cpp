//------------------------------------------
// Includes

// Standard library
#include <utility>

// External
#include <SDL3/SDL_render.h>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Core/Texture.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/DrawType/Sprite.hpp"

//------------------------------------------
namespace Nebulite::Graphics::DrawType {


Sprite::Sprite(Data::JsonScope& scope, DrawcallRefs& /*refs*/) {
    // Get Texture from container via link
    link = scope.get<std::string>(Key::imageLocation).value_or("");
}

void Sprite::drawToTexture(Core::Renderer& renderer, Core::Texture& texture, Data::JsonScope& scope, DrawcallRefs& /*refs*/) {
    if (link.empty()) {
        texture.capture.error.println("Sprite drawcall has empty texture link.");
        return;
    }

    // Set Source Rect, if it does not exist yet
    if (auto* const sdlTexture = renderer.getTexture(link); sdlTexture) {
        float w = 0.0;
        float h = 0.0;
        SDL_GetTextureSize(sdlTexture, &w, & h);

        // Setup src values unless they are already defined
        if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcX) != Data::KeyType::value) {
            scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcX, 0.0);
        }
        if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcY) != Data::KeyType::value) {
            scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcY, 0.0);
        }
        if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcW) != Data::KeyType::value) {
            scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcW, static_cast<double>(w) * 1.0);
        }
        if (scope.memberType(Constants::KeyNames::Drawcall::Rect::srcH) != Data::KeyType::value) {
            scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcH, static_cast<double>(h) * 1.0);
        }

        // Linked externally, as it's managed by the texture container
        texture.linkExternalTexture(sdlTexture);
    }
}

bool Sprite::diff(Data::JsonScope& scope, DrawcallRefs& /*refs*/) {
    return link != scope.get<std::string>(Key::imageLocation).value_or("");
}

std::pair<float,float> Sprite::getRenderOffset(DrawcallRefs& /*refs*/) {
    return std::make_pair(0.0, 0.0);
}

} // namespace Nebulite::Graphics::DrawType
