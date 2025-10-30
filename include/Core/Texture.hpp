/**
 * @file Texture.hpp
 * 
 * Proposed new Domain for Nebulite to bundle all texture related
 * functionality.
 */

#ifndef NEBULITE_CORE_TEXTURE_HPP
#define NEBULITE_CORE_TEXTURE_HPP

//------------------------------------------
// Includes

// External
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// Nebulite
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of core class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite::Core {
NEBULITE_DOMAIN(Texture){
public:
    /**
     * @brief Constructs a new Texture domain.
     * 
     * @param doc Pointer to the JSON document for storing texture properties.
     * @param globalSpace Pointer to the GlobalSpace for accessing the renderer.
     * @param renderer Pointer to the SDL_Renderer for texture operations.
     * @param texture Optional pointer to an existing SDL_Texture.
     */
    Texture(Nebulite::Utility::JSON* doc, Nebulite::Core::GlobalSpace* globalSpace);

    /**
     * @brief Destroys the Texture and frees resources.
     */
    ~Texture(){
        // Only destroy the texture if it was modified
        // And thus a local copy exists
        if(texture != nullptr && textureStoredLocally){
            SDL_DestroyTexture(texture);
        }
    }

    /**
     * @brief Updates the texture.
     */
    Nebulite::Constants::Error update() override;

    /**
     * @brief Necessary operations before parsing commands.
     */
    Nebulite::Constants::Error preParse() override;

    //------------------------------------------
    // SDL_Texture related

    /**
     * @brief Links an external SDL_Texture to this domain.
     * 
     * @param externalTexture Pointer to the external SDL_Texture.
     */
    void linkExternalTexture(SDL_Texture* externalTexture){
        texture = externalTexture;
        textureStoredLocally = false; // Reset modification flag
    }

    void setInternalTexture(SDL_Texture* newTexture){
        // Destroy any old internal texture if it was modified
        if (texture != nullptr && textureStoredLocally){
            SDL_DestroyTexture(texture);
        }
        texture = newTexture;
        textureStoredLocally = true; // Mark as modified since it's a new internal texture
    }

    /**
     * @brief Checks if the texture has been modified.
     * 
     * @return true if the texture has been modified, false otherwise.
     */
    bool isTextureStoredLocally() const noexcept {
        return textureStoredLocally;
    }

    /**
     * @brief Checks if the texture is valid (not null).
     * 
     * @return true if the texture is valid, false otherwise.
     */
    bool isTextureValid(){
        return texture != nullptr;
    }

    /**
     * @brief Gets the current SDL_Texture.
     * 
     * @return Pointer to the current SDL_Texture.
     */
    SDL_Texture* getSDLTexture(){
        return texture;
    }

    void loadTextureFromFile(std::string const& filePath);
private:
    /**
     * @brief Pointer to the linked globalspace.
     */
    Nebulite::Core::GlobalSpace* globalSpace;

    /**
     * @brief The SDL texture managed by this class.
     * If the texture is unmodified, this will reference the renderer's texture.
     * If modified, it will be a separate texture.
     */
    SDL_Texture* texture;

    // Flag to indicate if the texture is locally stored
    bool textureStoredLocally = false;

    /**
     * @brief Makes a copy of the texture currently managed by this class.
     * 
     * @return true if the copy was successful, false otherwise.
     */
    bool copyTexture();
};
}   // namespace Nebulite::Core
#endif // NEBULITE_CORE_TEXTURE_HPP