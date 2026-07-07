//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Core/Texture.hpp"
#include "Nebulite/Graphics/Drawcall.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Domain/Initializer.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Core {

Texture::Texture(Data::JsonScope& documentReference, Utility::IO::Capture& parentCapture)
    : Domain("Texture", documentReference, parentCapture) {
    // Initialize all DomainModules
    Module::Domain::Initializer::initTexture(this);
}

Constants::Event Texture::update() {
    updateModules();
    parseTaskQueues(true);

    // No evaluation of previous lines for now, just return NONE
    return Constants::Event::Success;
}

namespace {
SDL_Texture* copySdlTexture(SDL_Texture* source, SDL_Renderer* renderer, Utility::IO::Capture& capture) {
    auto* const currentTarget = SDL_GetRenderTarget(renderer);

    // Query
    float fw = 0.0f;
    float fh = 0.0f;
    if (!SDL_GetTextureSize(source, &fw, &fh)) {
        capture.error.println("Failed to query texture: ", SDL_GetError());
        return nullptr;
    }
    int const w = static_cast<int>(fw);
    int const h = static_cast<int>(fh);

    // Copy
    SDL_Texture* newTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!newTexture) {
        capture.error.println("Failed to create new texture: ", SDL_GetError());
        return nullptr;
    }
    if (!SDL_SetRenderTarget(renderer, newTexture)) {
        capture.error.println("Failed to set render texture: ", SDL_GetError());
        SDL_DestroyTexture(newTexture);
        return nullptr;
    }
    if (!SDL_RenderTexture(renderer, source, nullptr, nullptr)) {
        capture.error.println("Failed to copy texture: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, nullptr);
        SDL_DestroyTexture(newTexture);
        return nullptr;
    }
    SDL_SetRenderTarget(renderer, currentTarget);
    return newTexture;
}
} // namespace

void Texture::generateLocallyManagedTexture() {
    // Release the old texture if it was stored locally
    if (texture && textureStoredLocally) {
        SDL_DestroyTexture(texture);
    }

    // Get global texture
    std::string const& imageLink = domainScope.get<std::string>(Graphics::Drawcall::Key::SpriteSpecific::imageLocation).value_or("");
    auto* const globalTexture = Global::instance().getRenderer().getTexture(imageLink);
    if (globalTexture == nullptr) {
        capture.error.println("Failed to find texture in global renderer for image link: ", imageLink);
        return; // Could not find the texture in the global renderer, cannot proceed
    }

    // Copy
    texture = copySdlTexture(globalTexture, Global::instance().getSdlRenderer(), capture);
    if (texture == nullptr) {
        capture.error.println("Failed to copy texture for local management.");
        return;
    }
    textureStoredLocally = true;
}

void Texture::loadTextureFromFile(std::string const& filePath) {
    // Load the texture using the global renderer
    if (SDL_Texture* newTexture = Global::instance().getRenderer().loadTextureToMemory(filePath); newTexture) {
        // If a texture already exists and is stored locally, destroy it
        if (textureStoredLocally && texture) {
            SDL_DestroyTexture(texture);
        }
        texture = newTexture;
        textureStoredLocally = false; // New texture is not yet modified
    } else {
        capture.error.println("Failed to load texture from file: ", filePath);
    }
}

Constants::Event Texture::preParse() {
    if (!textureStoredLocally) {
        generateLocallyManagedTexture();
    }

    if (!textureStoredLocally) {
        // Failed to copy texture, cannot proceed with modifications
        return Constants::StandardCapture::Error::Texture::copyFailed(capture);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Core
