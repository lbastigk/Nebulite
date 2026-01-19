#include "DomainModule/RenderObject/Debug.hpp"

#include "Core/Renderer.hpp"
#include "Core/RenderObject.hpp"

#include "Nebulite.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update

Constants::Error Debug::update() {
    // For on-tick-updates
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

// NOLINTNEXTLINE
Constants::Error Debug::eval(std::span<std::string const> const& args, Interaction::Execution::DomainBase& caller, Data::JsonScopeBase& callerScope) {
    std::string const argStr = Utility::StringHandler::recombineArgs(args);
    Interaction::ContextBase const context{caller, caller, Global::instance()};    // Both self and other are this RenderObject?
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(argStr, context);
    (void)callerScope; // Unused
    return caller.parseStr(argsEvaluated);
}

Constants::Error Debug::printSrcRect() const {
    if (SDL_Rect const* srcRect = domain.getSrcRect(); srcRect) {
        std::string message;
        message += "Source Rectangle:";
        message += "{ x: " + std::to_string(srcRect->x);
        message += ", y: " + std::to_string(srcRect->y);
        message += ", w: " + std::to_string(srcRect->w);
        message += ", h: " + std::to_string(srcRect->h);
        message += " }";
        Log::println(message);
    } else {
        Log::println("This RenderObject is not a spritesheet.");
    }

    return Constants::ErrorTable::NONE();
}

Constants::Error Debug::printDstRect() const {
    if (SDL_Rect const* dstRect = domain.getDstRect(); dstRect) {
        std::string message;
        message += "Destination Rectangle:";
        message += "{ x: " + std::to_string(dstRect->x);
        message += ", y: " + std::to_string(dstRect->y);
        message += ", w: " + std::to_string(dstRect->w);
        message += ", h: " + std::to_string(dstRect->h);
        message += " }";
        Log::println(message);
    } else {
        Log::println("Destination rectangle is not set.");
    }

    return Constants::ErrorTable::NONE();
}

// Texture debugging helper
namespace {

std::string getTextureInfoString(Core::Renderer::TextureVariant const& textureVariant) {
    std::string info;

    if (std::holds_alternative<SDL_Texture*>(textureVariant)) {
        if (auto const texture = std::get<SDL_Texture*>(textureVariant); texture) {
            float w, h;
            if (SDL_GetTextureSize(texture, &w, &h) == 0) {
                // Print texture details
                info += " - Width  : " + std::to_string(w) + "\n";
                info += " - Height : " + std::to_string(h) + "\n";
            } else {
                info += "Failed to query texture: " + std::string(SDL_GetError());
            }
        } else {
            info += "No texture is associated with this RenderObject.";
        }
    }
    if (std::holds_alternative<SDL_GPUTexture*>(textureVariant)) {
        info += "Unable to retrieve GPU texture information.";
    }
    return info;
}


} // unnamed namespace

// NOLINTNEXTLINE
Constants::Error Debug::textureStatus(std::span<std::string const> const& /*args*/, Interaction::Execution::DomainBase& /*caller*/, Data::JsonScopeBase& callerScope) const {
    //------------------------------------------
    // Print Texture Status
    Log::println("Texture Status:");

    // Nebulite info
    Log::println(" - Texture Key   : " + callerScope.get<std::string>(Constants::KeyNames::RenderObject::imageLocation, "None"));
    Log::println(" - Valid Texture : " + std::string(domain.getTexture()->isTextureValid() ? "Yes" : "No"));
    Log::println(" - Local Texture : " + std::string(domain.getTexture()->isTextureStoredLocally() ? "Yes" : "No"));

    // SDL info
    Log::println("SDL Texture Info:");
    Log::println(getTextureInfoString(domain.getTexture()->getSDLTexture()));
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
