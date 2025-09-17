#include "Core/Texture.hpp"
#include "DomainModule/TDM.hpp"

#include "Core/GlobalSpace.hpp"

Nebulite::Core::Texture::Texture(Nebulite::Utility::JSON* doc, Nebulite::Core::GlobalSpace* globalSpace) 
: Nebulite::Interaction::Execution::Domain<Texture>("Texture",this,doc)
{
    texture  = nullptr; // Start with no texture
    this->globalSpace = globalSpace;
    Nebulite::DomainModule::TDM_init(this);
}

void Nebulite::Core::Texture::update() {
    for(auto& module : modules){
        module->update();
    }
}

bool Nebulite::Core::Texture::copyTexture() {
    if(texture != nullptr){
        // Get texture info
        int w, h;
        Uint32 format;
        int access;
        if(SDL_QueryTexture(texture, &format, &access, &w, &h) != 0){
            // Error querying texture
            return false;
        }

        // Create a new texture with the same properties
        SDL_Texture* newTexture = SDL_CreateTexture(globalSpace->getSDLRenderer(), format, access, w, h);
        if(!newTexture){
            // Error creating texture
            return false;
        }

        // Copy the content from the old texture to the new one
        SDL_SetRenderTarget(globalSpace->getSDLRenderer(), newTexture);
        SDL_RenderCopy(globalSpace->getSDLRenderer(), texture, NULL, NULL);
        SDL_SetRenderTarget(globalSpace->getSDLRenderer(), NULL);

        // Replace the old texture with the new one
        texture = newTexture;

        return true; // Successfully copied
    }
    return false; // No texture to copy
}

Nebulite::Constants::ERROR_TYPE Nebulite::Core::Texture::parseStr(const std::string& str) {
    if(!textureModified){
        // Make a local copy if we modify the texture
        textureModified = copyTexture(); 
    }

    if(!textureModified){
        // Failed to copy texture, cannot proceed with modifications
        return Nebulite::Constants::ERROR_TYPE::CRITICAL_GENERAL;
    }
    return funcTree->parseStr(str);
}