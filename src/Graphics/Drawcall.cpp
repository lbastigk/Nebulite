//------------------------------------------
// Includes

// Standard library
#include <cfloat>
#include <cmath>
#include <cstdint> // NOLINT
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_ttf/SDL_ttf.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/Renderer.hpp"
#include "Data/Document/JsonScope.hpp"
#include "Data/Document/KeyType.hpp"
#include "Graphics/Drawcall.hpp"
#include "Graphics/SdlPrimitive.hpp"
// NOLINTNEXTLINE
#include "Interaction/Context.hpp"
#include "Math/Equality.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/IdGenerator.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace {
uint64_t rollingJitter(uint32_t const& size) {
    static auto jitterGenerator = Nebulite::Utility::Coordination::IdGenerator::atomicThreadRollGenerator(size);
    return jitterGenerator();
}
} // namespace

//------------------------------------------
namespace Nebulite::Graphics {

Drawcall::Drawcall(Data::JsonScope& workspace, Utility::IO::Capture& parentCapture)
    : reInitializeRequested(true)
    , drawcallScope(workspace)
    , texture(workspace, parentCapture)
    , updaterRoutine{
        [this] {
            updateDrawcallData();
        },
        updateDrawcallDataIntervalMs + rollingJitter(updateDrawcallDataIntervalJitterMs),
        Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    } {
    refs.initialize(workspace);
    updateDrawcallData();
}

void Drawcall::Refs::initialize(Data::JsonScope const& scope){
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

    // Rotation
    rotationDegrees = scope.getStableDoublePointer(Key::rotationDegrees);
    rotationCenterX = scope.getStableDoublePointer(Key::rotationCenterX);
    rotationCenterY = scope.getStableDoublePointer(Key::rotationCenterY);

    // Color
    colorR = scope.getStableDoublePointer(Key::Color::R);
    colorG = scope.getStableDoublePointer(Key::Color::G);
    colorB = scope.getStableDoublePointer(Key::Color::B);
    colorA = scope.getStableDoublePointer(Key::Color::A);

    // Text-specific
    textFontsize = scope.getStableDoublePointer(Key::TextSpecific::fontsize);

    // Circle-specific
    circleRadius = scope.getStableDoublePointer(Key::CircleSpecific::radius);

    // Polygon-specific
    polygonFilled = scope.getStableDoublePointer(Key::PolygonSpecific::filled);
}

//------------------------------------------
// Rendering

void Drawcall::renderTexture(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY){
    if (texture.isTextureValid()) {
        SDL_FRect const srcRect = {
            .x=std::floor(static_cast<float>(*refs.rectSrcX)),
            .y=std::floor(static_cast<float>(*refs.rectSrcY)),
            .w=std::floor(static_cast<float>(*refs.rectSrcW)),
            .h=std::floor(static_cast<float>(*refs.rectSrcH))
        };
        SDL_FRect const dstRect = nebuliteRenderer.scaleRectFromLogicalSize({
            .x=std::floor(static_cast<float>(*refs.rectDstX) + dX),
            .y=std::floor(static_cast<float>(*refs.rectDstY) + dY),
            .w=std::floor(static_cast<float>(*refs.rectDstW)),
            .h=std::floor(static_cast<float>(*refs.rectDstH))
        });
        if (!Math::isZero(*refs.rotationDegrees)) {
            if (!SDL_RenderTextureRotated(nebuliteRenderer.getSdlRenderer(),texture.getSDLTexture(),&srcRect,&dstRect, *refs.rotationDegrees, &rotationCenter,SDL_FLIP_NONE)) {
                texture.capture.error.println("Failed to render rotated sprite texture in drawcall: ", SDL_GetError());
            }
        }
        else {
            if (!SDL_RenderTexture(nebuliteRenderer.getSdlRenderer(), texture.getSDLTexture(), &srcRect, &dstRect)) {
                texture.capture.error.println("Failed to render sprite texture in drawcall: ", SDL_GetError());
            }
        }
    }
    else {
        texture.capture.error.println("Attempted to draw uninitialized texture in drawcall.");
    }
}

void Drawcall::renderText(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY){
    // TODO: Why is the update needed for texts???
    //       After the first TimedRoutine trigger, the text drawcalls
    //       do not render properly unless we call updateDrawcallData continuously...
    if (reInitializeRequested) {
        initializeText();
        reInitializeRequested = false;
    }
    renderTexture(nebuliteRenderer, dX, dY);
}

void Drawcall::renderSprite(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY){
    if (reInitializeRequested) {
        initializeSprite();
        reInitializeRequested = false;
    }
    renderTexture(nebuliteRenderer, dX, dY);
}

void Drawcall::renderCircle(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY){
    if (reInitializeRequested) {
        initializeCircle();
        reInitializeRequested = false;
    }
    // Make sure the circle is centered
    auto const additionalOffsetX = static_cast<float>(*refs.rectDstW / 2.0);
    auto const additionalOffsetY = static_cast<float>(*refs.rectDstH / 2.0);
    renderTexture(nebuliteRenderer, dX - additionalOffsetX, dY - additionalOffsetY);
}

void Drawcall::renderPolygon(Core::Renderer const& nebuliteRenderer, float const& dX, float const& dY){
    if (reInitializeRequested) {
        initializePolygon();
        reInitializeRequested = false;
    }
    renderTexture(nebuliteRenderer, dX, dY);
}

void Drawcall::draw(Core::Renderer const& nebuliteRenderer, float const& offsetX, float const& offsetY) {
    switch (type) {
        // Sprite and text draw calls simply render their texture
        case Type::TEXT:
            renderText(nebuliteRenderer, offsetX, offsetY);
            break;
        case Type::SPRITE:
            renderSprite(nebuliteRenderer, offsetX, offsetY);
            break;
        case Type::CIRCLE:
            renderCircle(nebuliteRenderer, offsetX, offsetY);
            break;
        case Type::POLYGON:
            renderPolygon(nebuliteRenderer, offsetX, offsetY);
            break;
        default:
            std::unreachable();
    }
}

//------------------------------------------
// Updates

void Drawcall::update() {
    updaterRoutine.update();
    Global::instance().notifyEvent(texture.update());
}

void Drawcall::updateDrawcallData() {
    // TODO: Add more complicated diff-based update logic if needed
    //       Otherwise the current implementation re-initializes the texture with every routine trigger
    //       Perhaps some basic checks such as "has the text string/font size changed" would be sufficient for now
    //       However, any change in the drawcall data must be reflected here, otherwise the drawcall will not update properly!!
    if (auto const t = drawcallScope.get<std::string>(Key::type).value_or("sprite"); t == "sprite") {
        type = Type::SPRITE;
    }
    else if (t == "text") {
        type = Type::TEXT;
    }
    else if (t == "circle") {
        type = Type::CIRCLE;
    }
    else if (t == "polygon") {
        type = Type::POLYGON;
    }
    else {
        texture.capture.error.println("Unknown drawcall type: ", t, ". Defaulting to sprite.");
        type = Type::SPRITE;
    }

    // Setup rotation center
    if (drawcallScope.memberType(Key::rotationCenterX) == Data::KeyType::value) {
        rotationCenter.x = static_cast<float>(*refs.rectDstW * *refs.rotationCenterX);
    }
    else {
        // Default to center of dst rect
        rotationCenter.x = static_cast<float>(*refs.rectDstW / 2.0);
    }
    if (drawcallScope.memberType(Key::rotationCenterY) == Data::KeyType::value) {
        rotationCenter.y = static_cast<float>(*refs.rectDstH * *refs.rotationCenterY);
    }
    else {
        // Default to center of dst rect
        rotationCenter.y = static_cast<float>(*refs.rectDstH / 2.0);
    }

    // Force re-initialization on next draw
    reInitializeRequested = true;
}

//------------------------------------------
// Drawcall defaults

Constants::Event Drawcall::parseStr(std::string_view const str, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const {
    return texture.parseStr(str, ctx, ctxScope);
}

void Drawcall::ApplyDefault::Sprite(Data::JsonScope& scope) {
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

void Drawcall::ApplyDefault::Text(Data::JsonScope& scope) {
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

namespace {



}



void Drawcall::initializeSprite() {
    // Skip if renderer is not initialized
    if (!Global::instance().getRenderer().isSdlInitialized()) {
        return;
    }

    // Get Texture from container via link
    std::string const link = drawcallScope.get<std::string>(Key::SpriteSpecific::imageLocation).value_or("");
    if (link.empty()) {
        texture.capture.error.println("Sprite drawcall has empty texture link.");
        return;
    }

    // Set Source Rect, if it does not exist yet
    if (auto* const sdlTexture = Global::instance().getRenderer().getTexture(link); sdlTexture) {
        float w = 0.0;
        float h = 0.0;
        SDL_GetTextureSize(sdlTexture, &w, & h);

        // Setup src values unless they are already defined
        if (drawcallScope.memberType(Key::Rect::srcX) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcX, 0.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcY) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcY, 0.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcW) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcW, static_cast<double>(w) * 1.0);
        }
        if (drawcallScope.memberType(Key::Rect::srcH) != Data::KeyType::value) {
            drawcallScope.set<double>(Key::Rect::srcH, static_cast<double>(h) * 1.0);
        }

        // Linked externally, as it's managed by the texture container
        texture.linkExternalTexture(sdlTexture);
    }
}

void Drawcall::setStandardTextRectsIfMissing(float const w, float const h, TTF_Font* font) const {
    // Cast to double
    double const srcW = static_cast<double>(w) * 1.0;
    double const srcH = static_cast<double>(h) * 1.0;
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
}

void Drawcall::initializeText() {
    if (!Global::instance().getRenderer().isSdlInitialized()) return;

    SDL_Renderer* sdl = Global::instance().getRenderer().getSdlRenderer();
    if (!sdl) {
        texture.capture.error.println("Renderer not available for text drawcall.");
        return;
    }

    TTF_Font* font = Global::instance().getRenderer().getStandardFont();
    if (!font) {
        texture.capture.error.println("Font not available for text drawcall.");
        return;
    }

    // TODO: Proper width wrapping based on fontsize and max width
    auto text = drawcallScope.get<std::string>(Key::TextSpecific::str).value_or("");
    if (text.empty()) {
        text = " "; // Render at least a space to get height
    }

    SDL_Color const textColor = {
        .r=static_cast<Uint8>(*refs.colorR),
        .g=static_cast<Uint8>(*refs.colorG),
        .b=static_cast<Uint8>(*refs.colorB),
        .a=static_cast<Uint8>(*refs.colorA)
    };

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
    setStandardTextRectsIfMissing(w, h, font);
    texture.setInternalTexture(tex);
}

void Drawcall::initializeCircle() {
    // Set renderer to draw the circle
    SDL_Renderer* sdlRenderer = Global::instance().getRenderer().getSdlRenderer();
    if (!sdlRenderer) {
        texture.capture.error.println("Renderer not available for circle drawcall.");
        return;
    }

    // Create a texture for the circle
    int const radius = static_cast<int>(*refs.circleRadius);
    SDL_Texture* circleTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, radius * 2, radius * 2);

    // Draw the circle
    SDL_Color const circleColor = {
        .r=static_cast<Uint8>(*refs.colorR),
        .g=static_cast<Uint8>(*refs.colorG),
        .b=static_cast<Uint8>(*refs.colorB),
        .a=static_cast<Uint8>(*refs.colorA)
    };
    SDL_SetRenderDrawColor(sdlRenderer, circleColor.r, circleColor.g, circleColor.b, circleColor.a);
    SdlPrimitive::drawFilledCircle(sdlRenderer, circleTexture, circleColor, radius);

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

void Drawcall::initializePolygon() {
    // Set renderer to draw the polygon
    SDL_Renderer* sdlRenderer = Global::instance().getRenderer().getSdlRenderer();
    if (!sdlRenderer) {
        texture.capture.error.println("Renderer not available for circle drawcall.");
        return;
    }

    // Determine polygon size
    double const w = *refs.rectSrcW;
    double const h = *refs.rectSrcH;
    if (w < DBL_EPSILON || h < DBL_EPSILON) {
        texture.capture.error.println("Polygon drawcall has invalid src rect size. w and h must be > 0.");
        return;
    }

    // Setup destination rect if not already defined
    if (drawcallScope.memberType(Key::Rect::dstX) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstX, 0.0);
    }
    if (drawcallScope.memberType(Key::Rect::dstY) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstY, 0.0);
    }
    if (drawcallScope.memberType(Key::Rect::dstW) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstW, w);
    }
    if (drawcallScope.memberType(Key::Rect::dstH) != Data::KeyType::value) {
        drawcallScope.set<double>(Key::Rect::dstH, h);
    }

    // Get polygon points
    std::vector<SDL_FPoint> points;
    size_t const pointCount = drawcallScope.memberSize(Key::PolygonSpecific::points);
    if (pointCount < 2) { // Bump to 3 later on for filled polygons
        texture.capture.error.println("Polygon drawcall requires at least 2 points.");
        return;
    }
    points.reserve(pointCount);
    for (size_t i = 0; i < pointCount; ++i) {
        auto const pointX = w * drawcallScope.get<double>(Key::PolygonSpecific::points.addIndex(i).addMember("x")).value_or(0.0);
        auto const pointY = h * drawcallScope.get<double>(Key::PolygonSpecific::points.addIndex(i).addMember("y")).value_or(0.0);
        points.push_back({ .x=static_cast<float>(pointX), .y=static_cast<float>(pointY) });
    }

    // Create a texture for the polygon
    SDL_Texture* polyTexture = SDL_CreateTexture(
        sdlRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        static_cast<int>(w),
        static_cast<int>(h)
    );
    SDL_Color const polyColor = {
        .r=static_cast<Uint8>(*refs.colorR),
        .g=static_cast<Uint8>(*refs.colorG),
        .b=static_cast<Uint8>(*refs.colorB),
        .a=static_cast<Uint8>(*refs.colorA)
    };

    if (!Math::isZero(*refs.polygonFilled)) {
        // Filled polygon
        SdlPrimitive::drawFilledPolygon(sdlRenderer, polyTexture, polyColor, points);
    }
    else {
        // Empty polygon
        SdlPrimitive::drawEmptyPolygon(sdlRenderer, polyTexture, polyColor, points);
    }

    // Check for errors
    if (!polyTexture) {
        texture.capture.error.println("Failed to create polygon texture: ", SDL_GetError());
        return;
    }
    texture.linkExternalTexture(polyTexture);
}

} // namespace Nebulite::Graphics
