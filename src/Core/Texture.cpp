#include "Core/Texture.hpp"
#include "DomainModule/TXDM.hpp"

#include "Core/GlobalSpace.hpp"

Nebulite::Core::Texture::Texture(Nebulite::Utility::JSON* doc, Nebulite::Core::GlobalSpace* globalSpace) 
: Nebulite::Interaction::Execution::Domain<Texture>("Texture",this,doc), globalSpace(globalSpace)
{
    // Start with no texture
    texture  = nullptr;

    // Set preParse function
    setPreParse(std::bind(&Nebulite::Core::Texture::preParse,this));

    // Initialize all DomainModules
    Nebulite::DomainModule::TXDM_init(this);
}

Nebulite::Constants::Error Nebulite::Core::Texture::update() {
    updateModules();

    // No evaluation of previous lines for now, just return NONE
    return Nebulite::Constants::ErrorTable::NONE();
}

bool Nebulite::Core::Texture::copyTexture() {
    // If no texture is linked, try to load from the document
    if (texture == nullptr) {
        std::string imageLocation = Nebulite::Constants::keyName.renderObject.imageLocation;
        texture = globalSpace->getRenderer()->loadTextureToMemory(getDoc()->get<std::string>(imageLocation.c_str(),""));

        if(texture == nullptr) {
            return false; // No texture to copy
        }
    }

    // Get texture info
    int w, h;
    Uint32 format;
    int access;
    if (SDL_QueryTexture(texture, &format, &access, &w, &h) != 0) {
        std::cerr << "Failed to query texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create a new texture with streaming access for modifications
    SDL_Texture* newTexture = SDL_CreateTexture(globalSpace->getSDLRenderer(), format, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!newTexture) {
        std::cerr << "Failed to create new texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // Copy the content from the old texture to the new one
    if (SDL_SetRenderTarget(globalSpace->getSDLRenderer(), newTexture) != 0) {
        std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(newTexture);
        return false;
    }

    if (SDL_RenderCopy(globalSpace->getSDLRenderer(), texture, nullptr, nullptr) != 0) {
        std::cerr << "Failed to copy texture: " << SDL_GetError() << std::endl;
        SDL_SetRenderTarget(globalSpace->getSDLRenderer(), nullptr);
        SDL_DestroyTexture(newTexture);
        return false;
    }

    SDL_SetRenderTarget(globalSpace->getSDLRenderer(), nullptr);

    // Replace the old texture with the new one
    // We do not destroy the old texture, as it might be managed externally
    texture = newTexture;
    textureStoredLocally = true;
    return true; // Successfully copied
}

void Nebulite::Core::Texture::loadTextureFromFile(const std::string& filePath) {
    // Load the texture using the global renderer
    SDL_Texture* newTexture = globalSpace->getRenderer()->loadTextureToMemory(filePath);
    if (newTexture) {
        // If a texture already exists and is stored locally, destroy it
        if (textureStoredLocally && texture) {
            SDL_DestroyTexture(texture);
        }
        texture = newTexture;
        textureStoredLocally = false; // New texture is not yet modified
    } else {
        std::cerr << "Failed to load texture from file: " << filePath << std::endl;
    }
}

Nebulite::Constants::Error Nebulite::Core::Texture::preParse() {
    if(!textureStoredLocally){
        // Make a local copy if we modify the texture
        textureStoredLocally = copyTexture(); 
    }

    if(!textureStoredLocally){
        // Failed to copy texture, cannot proceed with modifications
        return Nebulite::Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_COPY_FAILED();
    }
    return Nebulite::Constants::ErrorTable::NONE();
}