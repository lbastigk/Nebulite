//------------------------------------------
// Includes

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Nebulite.hpp"
#include "Core/Texture.hpp"
#include "Graphics/Drawcall.hpp"
#include "DomainModule/Initializer.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Core {

Texture::Texture(JsonScope& documentReference)
    : Domain("Texture", *this, documentReference) {
    // Start with no texture
    texture = nullptr;

    // Initialize all DomainModules
    DomainModule::Initializer::initTexture(this);
}

Constants::Error Texture::update() {
    updateModules();

    // No evaluation of previous lines for now, just return NONE
    return Constants::ErrorTable::NONE();
}

bool Texture::copyTexture() {
    // If no texture is linked, try to load from the document
    if (texture == nullptr) {
        std::string const& imageLink = domainScope.get<std::string>(Graphics::Drawcall::Key::SpriteSpecific::imageLocation, "");
        texture = Global::instance().getRenderer().getTexture(imageLink);

        if (texture == nullptr) {
            return false; // No texture to copy
        }
    }
    // SDL3: query size, access and format using the new helpers
    float fw = 0.0f, fh = 0.0f;
    if (SDL_GetTextureSize(texture, &fw, &fh) != 0) {
        Error::println("Failed to query texture: ", SDL_GetError());
        return false;
    }
    int const w = static_cast<int>(fw);
    int const h = static_cast<int>(fh);

    // Create a new streaming texture (preserve format)
    SDL_Texture* newTexture = SDL_CreateTexture(Global::instance().getSdlRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!newTexture) {
        Error::println("Failed to create new texture: ", SDL_GetError());
        return false;
    }

    // Bind the new texture as the render target and copy
    if (SDL_SetRenderTarget(Global::instance().getSdlRenderer(), newTexture) != 0) {
        Error::println("Failed to set render texture: ", SDL_GetError());
        SDL_DestroyTexture(newTexture);
        return false;
    }

    if (SDL_RenderTexture(Global::instance().getSdlRenderer(), texture, nullptr, nullptr) != 0) {
        Error::println("Failed to copy texture: ", SDL_GetError());
        SDL_SetRenderTarget(Global::instance().getSdlRenderer(), nullptr);
        SDL_DestroyTexture(newTexture);
        return false;
    }

    // Unbind
    SDL_SetRenderTarget(Global::instance().getSdlRenderer(), nullptr);

    // Replace the old texture with the new one
    // We do not destroy the old texture, as it might be managed externally
    texture = newTexture;
    textureStoredLocally = true;
    return true; // Successfully copied
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
        Error::println("Failed to load texture from file: ", filePath);
    }
}

Constants::Error Texture::preParse() {
    if (!textureStoredLocally) {
        // Make a local copy if we modify the texture
        textureStoredLocally = copyTexture();
    }

    if (!textureStoredLocally) {
        // Failed to copy texture, cannot proceed with modifications
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_COPY_FAILED();
    }
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::Core
