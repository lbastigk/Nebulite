//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/Texture.hpp"
#include "Graphics/Drawcall.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Module/Domain/Initializer.hpp"
#include "Nebulite.hpp"
#include "Utility/IO/Capture.hpp"

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

void Texture::copyTexture() {
    // If no texture is linked, try to load from the document
    if (texture == nullptr) {
        std::string const& imageLink = domainScope.get<std::string>(Graphics::Drawcall::Key::SpriteSpecific::imageLocation).value_or("");
        texture = Global::instance().getRenderer().getTexture(imageLink);

        if (texture == nullptr) {
            return; // No texture to copy
        }
    }

    // SDL3: query size, access and format using the new helpers
    float fw = 0.0f;
    float fh = 0.0f;
    if (!SDL_GetTextureSize(texture, &fw, &fh)) {
        capture.error.println("Failed to query texture: ", SDL_GetError());
        return;
    }
    int const w = static_cast<int>(fw);
    int const h = static_cast<int>(fh);

    // Create a new texture
    SDL_Texture* newTexture = SDL_CreateTexture(Global::instance().getSdlRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if (!newTexture) {
        capture.error.println("Failed to create new texture: ", SDL_GetError());
        return;
    }

    // Bind the new texture as the render target and copy
    if (!SDL_SetRenderTarget(Global::instance().getSdlRenderer(), newTexture)) {
        capture.error.println("Failed to set render texture: ", SDL_GetError());
        SDL_DestroyTexture(newTexture);
        return;
    }

    if (!SDL_RenderTexture(Global::instance().getSdlRenderer(), texture, nullptr, nullptr)) {
        capture.error.println("Failed to copy texture: ", SDL_GetError());
        SDL_SetRenderTarget(Global::instance().getSdlRenderer(), nullptr);
        SDL_DestroyTexture(newTexture);
        return;
    }

    // Unbind
    SDL_SetRenderTarget(Global::instance().getSdlRenderer(), nullptr);

    // Replace the old texture with the new one
    // We do not destroy the old texture, as it might be managed externally
    texture = newTexture;
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
        copyTexture();
    }

    if (!textureStoredLocally) {
        // Failed to copy texture, cannot proceed with modifications
        return Constants::StandardCapture::Error::Texture::copyFailed(capture);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Core
