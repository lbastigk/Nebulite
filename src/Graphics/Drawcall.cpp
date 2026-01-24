#include "Nebulite.hpp"
#include "Graphics/Drawcall.hpp"

namespace Nebulite::Graphics {

void Drawcall::Refs::initialize(Core::JsonScope const& scope){
    // Source Rect
    rectSrcX = scope.getStableDoublePointer(Key::Rect::srcX);
    rectSrcY = scope.getStableDoublePointer(Key::Rect::srcY);
    rectSrcW = scope.getStableDoublePointer(Key::Rect::srcW);
    rectSrcH = scope.getStableDoublePointer(Key::Rect::srcH);

    // Destination Rect
    rectDstX = scope.getStableDoublePointer(Key::Rect::dstX);
    rectDstY = scope.getStableDoublePointer(Key::Rect::dstY);
    rectDstW = scope.getStableDoublePointer(Key::Rect::dstW);
    rectDstH = scope.getStableDoublePointer(Key::Rect::dstH);

    // Text-related
    textDx = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textDx);
    textDy = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textDy);
    textColorR = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textColorR);
    textColorG = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textColorG);
    textColorB = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textColorB);
    textColorA = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textColorA);
    textFontsize = scope.getStableDoublePointer(Constants::KeyNames::RenderObject::textFontsize);
}

void Drawcall::draw(float const& offsetX, float const& offsetY) const {
    switch (type) {
        case SPRITE:
            {
                if (SDL_Texture* sdlTexture = texture.getSDLTexture(); sdlTexture) {
                    SDL_FRect const dstRect = {
                        static_cast<float>(*refs.rectDstX) + offsetX,
                        static_cast<float>(*refs.rectDstY) + offsetY,
                        static_cast<float>(*refs.rectDstW),
                        static_cast<float>(*refs.rectDstH)
                    };
                    SDL_FRect const srcRect = {
                        static_cast<float>(*refs.rectSrcX),
                        static_cast<float>(*refs.rectSrcY),
                        static_cast<float>(*refs.rectSrcW),
                        static_cast<float>(*refs.rectSrcH)
                    };
                    SDL_RenderTexture(Global::instance().getSdlRenderer(), sdlTexture, &srcRect, &dstRect);
                }
            }
            break;
        case TEXT:
            {
                if (SDL_Texture* sdlTexture = texture.getSDLTexture(); sdlTexture) {
                    SDL_FRect const dstRect = {
                        static_cast<float>(*refs.rectDstX) + offsetX,
                        static_cast<float>(*refs.rectDstY) + offsetY,
                        static_cast<float>(*refs.rectDstW),
                        static_cast<float>(*refs.rectDstH)
                    };
                    SDL_RenderTexture(Global::instance().getSdlRenderer(), sdlTexture, nullptr, &dstRect);
                }
            }
            break;
        default:
            // Unknown type
            std::unreachable();
    }
}

void Drawcall::updateDrawcallData() {
    // TODO: Add more complicated update logic if needed
    if (auto const t = drawcallScope.get<std::string>(Key::type, "sprite"); t == "sprite") {
        type = SPRITE;
        initializeSprite();
    }
    else if (t == "text") {
        type = TEXT;
        initializeText();
    }
}

Constants::Error Drawcall::parseStr(std::string const& str) {
    return texture.parseStr(str);
}

//------------------------------------------
// Specific initializers

void Drawcall::initializeSprite() {
    // Delete old texture if stored locally
    if (texture.isTextureStoredLocally()) {
        if (SDL_Texture* old = texture.getSDLTexture(); old) {
            SDL_DestroyTexture(old);
        }
    }

    // Get Texture from container via link
    std::string const link = drawcallScope.get<std::string>(Key::Sprite::imageLocation);
    auto const sdlTexture = Global::instance().getRenderer().loadTextureToMemory(link);
    texture.linkExternalTexture(sdlTexture);

    // Setup rects based on info from JSON

    // Source Rect
    if (!drawcallScope.get<bool>(Key::Sprite::isSpritesheet)) {
        *refs.rectSrcX = static_cast<int>(drawcallScope.get<double>(Key::Sprite::spritesheetOffsetX));
        *refs.rectSrcY = static_cast<int>(drawcallScope.get<double>(Key::Sprite::spritesheetOffsetY));
        *refs.rectSrcW = static_cast<int>(drawcallScope.get<double>(Key::Sprite::spritesheetSizeX));
        *refs.rectSrcH = static_cast<int>(drawcallScope.get<double>(Key::Sprite::spritesheetSizeY));
    }
    else {
        *refs.rectSrcX = 0;
        *refs.rectSrcY = 0;
        *refs.rectSrcW = static_cast<int>(drawcallScope.get<double>(Key::Sprite::pixelSizeX));
        *refs.rectSrcH = static_cast<int>(drawcallScope.get<double>(Key::Sprite::pixelSizeY));
    }

    // Destination Rect
    *refs.rectDstX = static_cast<int>(drawcallScope.get<double>(Constants::KeyNames::RenderObject::positionX));
    *refs.rectDstY = static_cast<int>(drawcallScope.get<double>(Constants::KeyNames::RenderObject::positionY));
    *refs.rectDstW = static_cast<int>(drawcallScope.get<double>(Key::Sprite::pixelSizeX));
    *refs.rectDstH = static_cast<int>(drawcallScope.get<double>(Key::Sprite::pixelSizeY));
}

void Drawcall::initializeText() {
    // Delete old texture if stored locally
    if (texture.isTextureStoredLocally()) {
        if (SDL_Texture* old = texture.getSDLTexture(); old) {
            SDL_DestroyTexture(old);
        }
    }

    // Settings influenced by a new text
    auto const text = drawcallScope.get<std::string>(Key::Text::str);
    *refs.rectDstX = static_cast<int>(*refs.textDx);
    *refs.rectDstY = static_cast<int>(*refs.textDy);
    *refs.rectDstW = static_cast<int>(*refs.textFontsize * static_cast<double>(text.length()));
    *refs.rectDstH = static_cast<int>(*refs.textFontsize * 1.5);

    // Create text
    SDL_Color const textColor = {
        static_cast<Uint8>(*refs.textColorR),
        static_cast<Uint8>(*refs.textColorG),
        static_cast<Uint8>(*refs.textColorB),
        static_cast<Uint8>(*refs.textColorA)
    };

    // Create new texture
    SDL_Texture* sdlTexture = nullptr;
    if (!text.empty()) {
        TTF_Font* font = Global::instance().getRenderer().getStandardFont();
        if (SDL_Renderer* renderer = Global::instance().getSdlRenderer(); font && renderer) {
            if (SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), 0, textColor); textSurface) {
                sdlTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_DestroySurface(textSurface); // Free surface after creating texture
            }
        }
    }

    // Link texture
    texture.linkExternalTexture(sdlTexture);
}



} // namespace Nebulite::Graphics
