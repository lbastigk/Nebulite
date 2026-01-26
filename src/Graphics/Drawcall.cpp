//------------------------------------------
// Includes

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Nebulite.hpp"
#include "Graphics/Drawcall.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

Drawcall::Drawcall(Core::JsonScope& workspace) :
    drawcallScope(workspace),
    texture(workspace),
    updaterRoutine{
        [this] {
            updateDrawcallData();
        },
        updateDrawcallDataIntervalMs + std::rand() % updateDrawcallDataIntervalJitterMs,
        Utility::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    }
{
    reInitializeRequested = true;
    refs.initialize(workspace);
    updateDrawcallData();
}

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

    // Color
    colorR = scope.getStableDoublePointer(Key::Color::R);
    colorG = scope.getStableDoublePointer(Key::Color::G);
    colorB = scope.getStableDoublePointer(Key::Color::B);
    colorA = scope.getStableDoublePointer(Key::Color::A);

    // Text-specific
    textFontsize = scope.getStableDoublePointer(Key::TextSpecific::fontsize);

    // Circle-specific
    circleRadius = scope.getStableDoublePointer(Key::CircleSpecific::radius);
}

void Drawcall::draw(float const& offsetX, float const& offsetY) {
    // Helper lambda to render the texture
    auto renderTexture = [this](Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY) {
        if (texture.isTextureValid()) {
            SDL_FRect const srcRect = {
                std::floor(static_cast<float>(*refs.rectSrcX)),
                std::floor(static_cast<float>(*refs.rectSrcY)),
                std::floor(static_cast<float>(*refs.rectSrcW)),
                std::floor(static_cast<float>(*refs.rectSrcH))
            };
            SDL_FRect const dstRect = nebuliteRenderer.scaleRectFromLogicalSize({
                std::floor(static_cast<float>(*refs.rectDstX) + dX),
                std::floor(static_cast<float>(*refs.rectDstY) + dY),
                std::floor(static_cast<float>(*refs.rectDstW)),
                std::floor(static_cast<float>(*refs.rectDstH))
            });
            if (!SDL_RenderTexture(nebuliteRenderer.getSdlRenderer(), texture.getSDLTexture(), &srcRect, &dstRect)) {
                Error::println("Failed to render sprite texture in drawcall: ", SDL_GetError());
            }
        }
        else {
            Error::println("Attempted to draw uninitialized texture in drawcall.");
        }
    };

    auto const& renderer = Global::instance().getRenderer();
    switch (type) {
        // Sprite and text draw calls simply render their texture
        case TEXT:
            // TODO: Why is the update needed for texts???
            //       After the first TimedRoutine trigger, the text drawcalls
            //       do not render properly unless we call updateDrawcallData continuously...
            if (reInitializeRequested) {
                initializeText();
                reInitializeRequested = false;
            }
            renderTexture(renderer, offsetX, offsetY);
            break;
        case SPRITE:
            if (reInitializeRequested) {
                initializeSprite();
                reInitializeRequested = false;
            }
            renderTexture(renderer, offsetX, offsetY);
            break;
        // Later on, add more drawcall types here (geometry, etc.)
        case CIRCLE:
            if (reInitializeRequested) {
                initializeCircle();
                reInitializeRequested = false;
            }
            renderTexture(renderer, offsetX, offsetY);
            break;
        default:
            // Unknown type
            std::unreachable();
    }
}

void Drawcall::update() {
    updaterRoutine.update();
    texture.update();
}

void Drawcall::updateDrawcallData() {
    // TODO: Add more complicated diff-based update logic if needed
    //       Otherwise the current implementation re-initializes the texture with every routine trigger
    //       Perhaps some basic checks such as "has the text string/font size changed" would be sufficient for now
    //       However, any change in the drawcall data must be reflected here, otherwise the drawcall will not update properly!!
    if (auto const t = drawcallScope.get<std::string>(Key::type, "sprite"); t == "sprite") {
        type = SPRITE;
    }
    else if (t == "text") {
        type = TEXT;
    }
    else if (t == "circle") {
        type = CIRCLE;
    }
    else {
        Error::println("Unknown drawcall type: ", t, ". Defaulting to sprite.");
        type = SPRITE;
    }
    reInitializeRequested = true; // Force re-initialization on next draw
}

Constants::Error Drawcall::parseStr(std::string const& str) {
    return texture.parseStr(str);
}

void Drawcall::ApplyDefault::Sprite(Core::JsonScope& scope) {
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

void Drawcall::ApplyDefault::Text(Core::JsonScope& scope) {
    // Default type
    scope.set<std::string>(Key::type, "text");
    scope.set<std::string>(Key::TextSpecific::str, "Hello, Nebulite!");
    scope.set<double>(Key::TextSpecific::fontsize, 24.0);
    scope.set<double>(Key::Color::R, 255.0);
    scope.set<double>(Key::Color::G, 255.0);
    scope.set<double>(Key::Color::B, 255.0);
    scope.set<double>(Key::Color::A, 255.0);

    // Default Rects will be set during initialization based on text size
}

//------------------------------------------
// Specific initializers

void Drawcall::initializeSprite() {
    // Skip if renderer is not initialized
    if (!Global::instance().getRenderer().isSdlInitialized()) {
        return;
    }

    // Get Texture from container via link
    std::string const link = drawcallScope.get<std::string>(Key::SpriteSpecific::imageLocation);
    if (link.empty()) {
        Error::println("Sprite drawcall has empty texture link.");
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

        // Linked externally, as it's managed by the texture container
        texture.linkExternalTexture(sdlTexture);
    }
}

void Drawcall::initializeText() {
    if (!Global::instance().getRenderer().isSdlInitialized()) return;

    SDL_Renderer* sdl = Global::instance().getRenderer().getSdlRenderer();
    if (!sdl) {
        Error::println("Renderer not available for text drawcall.");
        return;
    }

    TTF_Font* font = Global::instance().getRenderer().getStandardFont();
    if (!font) {
        Error::println("Font not available for text drawcall.");
        return;
    }

    // TODO: Proper width wrapping based on fontsize and max width
    auto text = drawcallScope.get<std::string>(Key::TextSpecific::str);
    if (text.empty()) {
        text = " "; // Render at least a space to get height
    }

    SDL_Color const textColor = {
        static_cast<Uint8>(*refs.colorR),
        static_cast<Uint8>(*refs.colorG),
        static_cast<Uint8>(*refs.colorB),
        static_cast<Uint8>(*refs.colorA)
    };

    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(font, text.c_str(), 0, textColor, 0);
    if (!surf) {
        Error::println("TTF_RenderText_Blended_Wrapped failed: ", SDL_GetError());
        return;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl, surf);
    SDL_DestroySurface(surf);
    if (!tex) {
        Error::println("SDL_CreateTextureFromSurface failed: ", SDL_GetError());
        return;
    }

    float w = 0, h = 0;
    if (!SDL_GetTextureSize(tex, &w, &h)) {
        Error::println("SDL_GetTextureSize failed: ", SDL_GetError());
        SDL_DestroyTexture(tex);
        return;
    }

    // Cast to double
    double const srcW = static_cast<double>(w);
    double const srcH = static_cast<double>(h);
    double const dstW = srcW * *refs.textFontsize / static_cast<double>(TTF_GetFontSize(font));
    double const dstH = srcH * *refs.textFontsize / static_cast<double>(TTF_GetFontSize(font));

    // Setup src values unless they are already defined
    if (drawcallScope.memberType(Key::Rect::srcX) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::srcX, 0.0);
    if (drawcallScope.memberType(Key::Rect::srcY) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::srcY, 0.0);
    if (drawcallScope.memberType(Key::Rect::srcW) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::srcW, srcW);
    if (drawcallScope.memberType(Key::Rect::srcH) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::srcH, srcH);

    // Prefer measured pixel size for dst unless the caller explicitly set different values
    if (drawcallScope.memberType(Key::Rect::dstX) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::dstX, 0.0);
    if (drawcallScope.memberType(Key::Rect::dstY) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::dstY, 0.0);
    if (drawcallScope.memberType(Key::Rect::dstW) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::dstW, dstW);
    if (drawcallScope.memberType(Key::Rect::dstH) != Data::KeyType::value) drawcallScope.set<double>(Key::Rect::dstH, dstH);

    texture.setInternalTexture(tex);
}

// TODO: Move to Graphics/SdlPrimitives.hpp/cpp
namespace {
void drawFilledCircle(
    SDL_Renderer *renderer,
    int const cx,
    int const cy,
    int const radius
) {
    for (int dy = -radius; dy <= radius; dy++) {
        int const dx = static_cast<int>(sqrt(radius * radius - dy * dy));
        SDL_RenderLine(
            renderer,
            static_cast<float>(cx - dx), static_cast<float>(cy + dy),
            static_cast<float>(cx + dx), static_cast<float>(cy + dy)
        );
    }
}
} // anonymous namespace

void Drawcall::initializeCircle() {
    // Set renderer to draw the circle
    SDL_Renderer* sdlRenderer = Global::instance().getRenderer().getSdlRenderer();
    if (!sdlRenderer) {
        Error::println("Renderer not available for circle drawcall.");
        return;
    }

    // Create a texture for the circle
    int const radius = static_cast<int>(*refs.circleRadius);
    SDL_Texture* circleTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, radius * 2, radius * 2);

    // Set the texture as the rendering target
    SDL_SetRenderTarget(sdlRenderer, circleTexture);
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0); // Transparent background
    SDL_RenderClear(sdlRenderer);

    // Draw the circle
    SDL_Color const circleColor = {
        static_cast<Uint8>(*refs.colorR),
        static_cast<Uint8>(*refs.colorG),
        static_cast<Uint8>(*refs.colorB),
        static_cast<Uint8>(*refs.colorA)
    };
    SDL_SetRenderDrawColor(sdlRenderer, circleColor.r, circleColor.g, circleColor.b, circleColor.a);
    drawFilledCircle(sdlRenderer, radius, radius, radius);

    // DEBUG: Fill every pixel
    SDL_FRect const rect = { 0, 0, 2.0f*radius, 2.0f*radius };
    SDL_RenderRect(sdlRenderer, &rect);

    // Reset to default render target
    SDL_SetRenderTarget(sdlRenderer, nullptr);

    // Setup src values unless they are already defined
    if (drawcallScope.memberType(Key::Rect::srcX) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::srcX, 0.0);
    }
    if (drawcallScope.memberType(Key::Rect::srcY) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::srcY, 0.0);
    }
    if (drawcallScope.memberType(Key::Rect::srcW) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::srcW, 2*radius);
    }
    if (drawcallScope.memberType(Key::Rect::srcH) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::srcH, 2*radius);
    }
    texture.setInternalTexture(circleTexture);
}

} // namespace Nebulite::Graphics
