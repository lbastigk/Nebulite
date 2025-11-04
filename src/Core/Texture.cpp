//------------------------------------------
// Includes

// External
#include <SDL.h>

// Nebulite
#include "Constants/KeyNames.hpp"
#include "Core/GlobalSpace.hpp"
#include "Core/Texture.hpp"
#include "DomainModule/Initializer.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Core {

Texture::Texture(Utility::JSON* doc, GlobalSpace* globalSpace)
:   Domain("Texture", this, doc, globalSpace),
    globalSpace(globalSpace)
{
    // Start with no texture
    texture  = nullptr;

    // Set preParse function
    setPreParse([this] { return preParse(); });

    // Initialize all DomainModules
    DomainModule::Initializer::initTexture(this);
}

Constants::Error Texture::update(){
    updateModules();

    // No evaluation of previous lines for now, just return NONE
    return Constants::ErrorTable::NONE();
}

bool Texture::copyTexture(){
    // If no texture is linked, try to load from the document
    if (texture == nullptr){
        std::string const imageLocation = Constants::keyName.renderObject.imageLocation;
        texture = globalSpace->getRenderer()->loadTextureToMemory(getDoc()->get<std::string>(imageLocation,""));

        if(texture == nullptr){
            return false; // No texture to copy
        }
    }

    // Get texture info
    int w, h;
    Uint32 format;
    int textureAccess;
    if (SDL_QueryTexture(texture, &format, &textureAccess, &w, &h) != 0){
        Utility::Capture::cerr() << "Failed to query texture: " << SDL_GetError() << Utility::Capture::endl;
        return false;
    }

    // Create a new texture with streaming access for modifications
    SDL_Texture* newTexture = SDL_CreateTexture(globalSpace->getSdlRenderer(), format, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!newTexture){
        Utility::Capture::cerr() << "Failed to create new texture: " << SDL_GetError() << Utility::Capture::endl;
        return false;
    }

    // Copy the content from the old texture to the new one
    if (SDL_SetRenderTarget(globalSpace->getSdlRenderer(), newTexture) != 0){
        Utility::Capture::cerr() << "Failed to set render target: " << SDL_GetError() << Utility::Capture::endl;
        SDL_DestroyTexture(newTexture);
        return false;
    }

    if (SDL_RenderCopy(globalSpace->getSdlRenderer(), texture, nullptr, nullptr) != 0){
        Utility::Capture::cerr() << "Failed to copy texture: " << SDL_GetError() << Utility::Capture::endl;
        SDL_SetRenderTarget(globalSpace->getSdlRenderer(), nullptr);
        SDL_DestroyTexture(newTexture);
        return false;
    }

    SDL_SetRenderTarget(globalSpace->getSdlRenderer(), nullptr);

    // Replace the old texture with the new one
    // We do not destroy the old texture, as it might be managed externally
    texture = newTexture;
    textureStoredLocally = true;
    return true; // Successfully copied
}

void Texture::loadTextureFromFile(std::string const& filePath){
    // Load the texture using the global renderer
    if (SDL_Texture* newTexture = globalSpace->getRenderer()->loadTextureToMemory(filePath); newTexture){
        // If a texture already exists and is stored locally, destroy it
        if (textureStoredLocally && texture){
            SDL_DestroyTexture(texture);
        }
        texture = newTexture;
        textureStoredLocally = false; // New texture is not yet modified
    } else {
        Utility::Capture::cerr() << "Failed to load texture from file: " << filePath << Utility::Capture::endl;
    }
}

Constants::Error Texture::preParse(){
    if(!textureStoredLocally){
        // Make a local copy if we modify the texture
        textureStoredLocally = copyTexture();
    }

    if(!textureStoredLocally){
        // Failed to copy texture, cannot proceed with modifications
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_COPY_FAILED();
    }
    return Constants::ErrorTable::NONE();
}

}  // namespace Nebulite::Core