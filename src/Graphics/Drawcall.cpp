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
    textColorR = scope.getStableDoublePointer(Key::TextSpecific::colorR);
    textColorG = scope.getStableDoublePointer(Key::TextSpecific::colorG);
    textColorB = scope.getStableDoublePointer(Key::TextSpecific::colorB);
    textColorA = scope.getStableDoublePointer(Key::TextSpecific::colorA);
    textFontsize = scope.getStableDoublePointer(Key::TextSpecific::fontsize);
}

void Drawcall::draw(float const& offsetX, float const& offsetY) const {
    switch (type) {
        case SPRITE:
            {
                if (SDL_Texture* sdlTexture = texture.getSDLTexture(); sdlTexture) {
                    SDL_FRect const srcRect = {
                        static_cast<float>(*refs.rectSrcX),
                        static_cast<float>(*refs.rectSrcY),
                        static_cast<float>(*refs.rectSrcW),
                        static_cast<float>(*refs.rectSrcH)
                    };
                    SDL_FRect const dstRect = {
                        static_cast<float>(*refs.rectDstX) + offsetX,
                        static_cast<float>(*refs.rectDstY) + offsetY,
                        static_cast<float>(*refs.rectDstW),
                        static_cast<float>(*refs.rectDstH)
                    };
                    SDL_RenderTexture(Global::instance().getSdlRenderer(), sdlTexture, &srcRect, &dstRect);
                }
            }
            break;
        case TEXT:
            {
                if (SDL_Texture* sdlTexture = texture.getSDLTexture(); sdlTexture) {
                    SDL_FRect const srcRect = {
                        static_cast<float>(*refs.rectSrcX),
                        static_cast<float>(*refs.rectSrcY),
                        static_cast<float>(*refs.rectSrcW),
                        static_cast<float>(*refs.rectSrcH)
                    };
                    SDL_FRect const dstRect = {
                        static_cast<float>(*refs.rectDstX) + offsetX,
                        static_cast<float>(*refs.rectDstY) + offsetY,
                        static_cast<float>(*refs.rectDstW),
                        static_cast<float>(*refs.rectDstH)
                    };
                    // TODO: Draws black texture instead of text!
                    SDL_RenderTexture(Global::instance().getSdlRenderer(), sdlTexture, &srcRect, &dstRect);
                }
            }
            break;
        default:
            // Unknown type
            std::unreachable();
    }
}

void Drawcall::updateDrawcallData() {
    // TODO: Add more complicated diff-based update logic if needed
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

void Drawcall::setDefaultTypeSprite(Core::JsonScope& scope) {
    // Default type
    scope.set<std::string>(Key::type, "sprite");
    scope.set<std::string>(Key::SpriteSpecific::imageLocation, "Resources/Sprites/TEST001P/001.bmp");

    // Default Rects
    scope.set<double>(Key::Rect::srcX, 0.0);
    scope.set<double>(Key::Rect::srcY, 0.0);
    scope.set<double>(Key::Rect::srcW, 32.0);
    scope.set<double>(Key::Rect::srcH, 32.0);
    scope.set<double>(Key::Rect::dstX, 0.0);
    scope.set<double>(Key::Rect::dstY, 0.0);
    scope.set<double>(Key::Rect::dstW, 32.0);
    scope.set<double>(Key::Rect::dstH, 32.0);
}

void Drawcall::setDefaultTypeText(Core::JsonScope& scope) {
    // Default type
    scope.set<std::string>(Key::type, "text");
    scope.set<std::string>(Key::TextSpecific::str, "Hello, Nebulite!");
    scope.set<double>(Key::TextSpecific::fontsize, 24.0);
    scope.set<double>(Key::TextSpecific::colorR, 255.0);
    scope.set<double>(Key::TextSpecific::colorG, 255.0);
    scope.set<double>(Key::TextSpecific::colorB, 255.0);
    scope.set<double>(Key::TextSpecific::colorA, 255.0);

    // Default Rects will be set during initialization based on text size
}

//------------------------------------------
// Specific initializers

void Drawcall::initializeSprite() {
    // Skip if renderer is not initialized
    if (!Global::instance().getRenderer().isSdlInitialized()) {
        return;
    }

    // Delete old texture if stored locally
    if (texture.isTextureStoredLocally()) {
        if (SDL_Texture* old = texture.getSDLTexture(); old) {
            SDL_DestroyTexture(old);
        }
    }

    // Get Texture from container via link
    std::string const link = drawcallScope.get<std::string>(Key::SpriteSpecific::imageLocation);
    if (link.empty()) {
        return;
    }

    // Set Source Rect, if it does not exist yet
    if (auto const sdlTexture = Global::instance().getRenderer().getTexture(link); sdlTexture) {
        float w, h;
        SDL_GetTextureSize(sdlTexture, &w, & h);

        // Setup src values unless they are already defined
        if (drawcallScope.memberType(Key::Rect::srcX) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcX, 0.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcY) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcY, 0.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcW) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcW, static_cast<double>(w));
        }
        if (drawcallScope.memberType(Key::Rect::srcH) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcH, static_cast<double>(h));
        }

        texture.linkExternalTexture(sdlTexture);
        status.initialized = true;
    }
}

void Drawcall::initializeText() {
    // Skip if renderer is not initialized
    if (!Global::instance().getRenderer().isSdlInitialized()) {
        return;
    }

    // Delete old texture if stored locally
    if (texture.isTextureStoredLocally()) {
        if (SDL_Texture* old = texture.getSDLTexture(); old) {
            SDL_DestroyTexture(old);
        }
    }

    // Coloring


    // Create new texture
    SDL_Texture* sdlTexture = nullptr;
    auto const text = drawcallScope.get<std::string>(Key::TextSpecific::str);
    if (!text.empty()) {
        TTF_Font* font = Global::instance().getRenderer().getStandardFont();
        if (SDL_Renderer* renderer = Global::instance().getSdlRenderer(); font && renderer) {
            SDL_Color const textColor = {
                static_cast<Uint8>(*refs.textColorR),
                static_cast<Uint8>(*refs.textColorG),
                static_cast<Uint8>(*refs.textColorB),
                static_cast<Uint8>(*refs.textColorA)
            };
            if (SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), 0, textColor); textSurface) {
                sdlTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_DestroySurface(textSurface); // Free surface after creating texture
            }
        }
    }

    // Setup dst values unless they are already defined
    if (drawcallScope.memberType(Key::Rect::dstX) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstX, 0.0);
    }
    if (drawcallScope.memberType(Key::Rect::dstY) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstY, 0.0);
    }
    if (drawcallScope.memberType(Key::Rect::dstW) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstW, *refs.textFontsize * static_cast<double>(text.length()));
    }
    if (drawcallScope.memberType(Key::Rect::dstH) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstH, *refs.textFontsize * 1.5);
    }

    // Setup source rect, if it does not exist yet
    if (sdlTexture) {
        float w, h;
        SDL_GetTextureSize(sdlTexture, &w, & h);

        // Setup src values unless they are already defined
        if (drawcallScope.memberType(Key::Rect::srcX) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcX, 0.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcY) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcY, 0.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcW) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcW, static_cast<double>(w));
        }
        if (drawcallScope.memberType(Key::Rect::srcH) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcH, static_cast<double>(h));
        }

        // Link texture, mark as initialized
        texture.linkExternalTexture(sdlTexture);
        status.initialized = true;
    }
}

} // namespace Nebulite::Graphics
