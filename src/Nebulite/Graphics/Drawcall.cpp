//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <cmath>
#include <cstdint> // NOLINT
#include <memory>
#include <string>
#include <string_view>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Data/Document/KeyType.hpp"
#include "Nebulite/Graphics/DrawType/Circle.hpp"
#include "Nebulite/Graphics/DrawType/Polygon.hpp"
#include "Nebulite/Graphics/DrawType/Sprite.hpp"
#include "Nebulite/Graphics/DrawType/Text.hpp"
#include "Nebulite/Graphics/Drawcall.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Coordination/IdGenerator.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace {
template<uint32_t Size>
uint64_t rollingJitter() {
    static auto jitterGenerator = Nebulite::Utility::Coordination::IdGenerator::atomicRollingIdGenerator(Size);
    return jitterGenerator();
}
} // namespace

//------------------------------------------
namespace Nebulite::Graphics {

Drawcall::Drawcall(Data::JsonScope& workspace, Utility::Io::Capture& parentCapture)
    : texture(workspace, parentCapture)
    , drawcallScope(workspace)
    , updaterRoutine{
        [this] {
            updateDrawcallData();
        },
        updateDrawcallDataIntervalMs + rollingJitter<updateDrawcallDataIntervalJitterMs>(),
        Utility::Coordination::TimedRoutine::ConstructionMode::startImmediately
    }
    , reInitializeRequested(true) {
    refs.initialize(workspace);
    updateDrawcallData();
}

//------------------------------------------
// Rendering

void Drawcall::renderTexture(Core::Renderer const& nebuliteRenderer, float const dX, float const dY){
    if (texture.isTextureValid()) {
        SDL_FRect const srcRect = {
            .x=std::floor(static_cast<float>(*refs.rectSrcX)),
            .y=std::floor(static_cast<float>(*refs.rectSrcY)),
            .w=std::floor(static_cast<float>(*refs.rectSrcW)),
            .h=std::floor(static_cast<float>(*refs.rectSrcH)),
        };
        SDL_FRect const dstRect = nebuliteRenderer.scaleRectFromLogicalSize({
            .x=std::floor(static_cast<float>(*refs.rectDstX) + dX),
            .y=std::floor(static_cast<float>(*refs.rectDstY) + dY),
            .w=std::floor(static_cast<float>(*refs.rectDstW)),
            .h=std::floor(static_cast<float>(*refs.rectDstH)),
        });
        if (!Math::isZero(*refs.rotationDegrees)) {
            if (!SDL_RenderTextureRotated(nebuliteRenderer.getSdlRenderer(), texture.getSdlTexture(), &srcRect, &dstRect, *refs.rotationDegrees, &rotationCenter, SDL_FLIP_NONE)) {
                texture.capture.error.println("Failed to render rotated sprite texture in drawcall: ", SDL_GetError());
            }
        }
        else {
            if (!SDL_RenderTexture(nebuliteRenderer.getSdlRenderer(), texture.getSdlTexture(), &srcRect, &dstRect)) {
                texture.capture.error.println("Failed to render sprite texture in drawcall: ", SDL_GetError());
            }
        }
    }
    else {
        texture.capture.error.println("Attempted to draw uninitialized texture in drawcall.");
    }
}

void Drawcall::draw(Core::Renderer& nebuliteRenderer, float const offsetX, float const offsetY) {
    assert(toDraw != nullptr);

    // Re-draw texture if requested, or if values changed
    if (reInitializeRequested) {
        toDraw->drawToTexture(nebuliteRenderer, texture, drawcallScope, refs);
        reInitializeRequested = false;
    }

    // Render
    auto const [textureOffsetX, textureOffsetY] = toDraw->getRenderOffset(refs);
    renderTexture(nebuliteRenderer, offsetX + textureOffsetX, offsetY + textureOffsetY);
}

//------------------------------------------
// Updates

void Drawcall::update() {
    updaterRoutine.update();
    Global::instance().notifyEvent(texture.update());
}

void Drawcall::updateDrawcallData() {
    auto const typeSet = drawcallScope.get<std::string>(Constants::KeyNames::Drawcall::type);
    if (!typeSet.has_value()) {
        texture.capture.error.println("No type set. Defaulting to sprite.");
    }

    // Setup new type
    if (auto const t = typeSet.value_or("sprite"); t == "sprite") {
        if (type != Type::sprite || !toDraw) {
            reInitializeRequested = true;
            type = Type::sprite;
            toDraw = std::make_unique<DrawType::Sprite>(drawcallScope, refs);
        }
    }
    else if (t == "text") {
        if (type != Type::text || !toDraw) {
            reInitializeRequested = true;
            type = Type::text;
            toDraw = std::make_unique<DrawType::Text>(drawcallScope, refs);
        }
    }
    else if (t == "circle") {
        if (type != Type::circle || !toDraw) {
            reInitializeRequested = true;
            type = Type::circle;
            toDraw = std::make_unique<DrawType::Circle>(drawcallScope, refs);
        }
    }
    else if (t == "polygon") {
        if (type != Type::polygon || !toDraw) {
            reInitializeRequested = true;
            type = Type::polygon;
            toDraw = std::make_unique<DrawType::Polygon>(drawcallScope, refs);
        }
    }
    else {
        texture.capture.error.println("Unknown drawcall type: ", t, ". Defaulting to sprite.");
        type = Type::sprite;
        toDraw = std::make_unique<DrawType::Sprite>(drawcallScope, refs);
    }

    // Request reinit on diff
    if (toDraw->diff(drawcallScope, refs)) {
        reInitializeRequested = true;
    }

    // Setup rotation center
    if (drawcallScope.memberType(Constants::KeyNames::Drawcall::rotationCenterX) == Data::KeyType::value) {
        rotationCenter.x = static_cast<float>(*refs.rectDstW * *refs.rotationCenterX);
    }
    else {
        // Default to center of dst rect
        rotationCenter.x = static_cast<float>(*refs.rectDstW / 2.0);
    }
    if (drawcallScope.memberType(Constants::KeyNames::Drawcall::rotationCenterY) == Data::KeyType::value) {
        rotationCenter.y = static_cast<float>(*refs.rectDstH * *refs.rotationCenterY);
    }
    else {
        // Default to center of dst rect
        rotationCenter.y = static_cast<float>(*refs.rectDstH / 2.0);
    }
}

//------------------------------------------
// Drawcall defaults

Constants::Event Drawcall::parseStr(std::string_view const str, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const {
    return texture.parseStr(str, ctx, ctxScope);
}

void Drawcall::ApplyDefault::sprite(Data::JsonScope& scope) {
    // Default type
    scope.set<std::string>(Constants::KeyNames::Drawcall::type, "sprite");
    scope.set<std::string>(DrawType::Sprite::Key::imageLocation, "Resources/Sprites/TEST001P/001.bmp");

    // Default Rects
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcX, 0.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcY, 0.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcW, 32.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::srcH, 32.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstX, 0.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstY, 0.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstW, 32.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Rect::dstH, 32.0);
}

void Drawcall::ApplyDefault::text(Data::JsonScope& scope) {
    // Default type
    scope.set<std::string>(Constants::KeyNames::Drawcall::type, "text");
    scope.set<std::string>(DrawType::Text::Key::str, "Hello, Nebulite!");
    scope.set<double>(DrawType::Text::Key::fontsize, 24.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Color::r, 255.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Color::g, 255.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Color::b, 255.0);
    scope.set<double>(Constants::KeyNames::Drawcall::Color::a, 255.0);
}
} // namespace Nebulite::Graphics
