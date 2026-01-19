/**
 * @file Texture.hpp
 * @brief Texture management domain for Nebulite.
 */

#ifndef NEBULITE_CORE_TEXTURE_HPP
#define NEBULITE_CORE_TEXTURE_HPP

//------------------------------------------
// Includes

// External
#include <SDL3/SDL.h>

// Nebulite
#include "Core/Renderer.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite::Core {
NEBULITE_DOMAIN(Texture) {
public:
    /**
     * @brief Constructs a new Texture domain.
     * @param documentReference Reference to the JSON document for this domain.
     */
    explicit Texture(JsonScope& documentReference);

    /**
     * @brief Destroys the Texture and frees resources.
     */
    ~Texture() override ;

    /**
     * @brief Updates the texture.
     */
    Constants::Error update() override;

    /**
     * @brief Necessary operations before parsing commands.
     */
    Constants::Error preParse() override;

    //------------------------------------------
    // SDL_Texture related

    /**
     * @brief Links an external SDL_Texture to this domain.
     * @param externalTexture Pointer to the external SDL_Texture.
     */
    void linkExternalTexture(Renderer::TextureVariant const& externalTexture) {
        texture = externalTexture;
        textureStoredLocally = false; // Reset modification flag
    }

    /**
     * @brief Sets a new internal SDL_Texture, marking it as modified.
     * @param newTexture Pointer to the new SDL_Texture.
     */
    void setInternalTexture(Renderer::TextureVariant const& newTexture);

    /**
     * @brief Checks if the texture has been modified.
     * @return true if the texture has been modified, false otherwise.
     */
    [[nodiscard]] bool isTextureStoredLocally() const noexcept {
        return textureStoredLocally;
    }

    /**
     * @brief Checks if the texture is valid (not null).
     * @return true if the texture is valid, false otherwise.
     */
    [[nodiscard]] bool isTextureValid() const noexcept {
        return Renderer::isTextureValid(texture);
    }

    /**
     * @brief Gets the current SDL_Texture.
     * @return Pointer to the current SDL_Texture.
     */
    [[nodiscard]] Renderer::TextureVariant getSDLTexture() const noexcept {
        return texture;
    }

    void loadTextureFromFile(std::string const& filePath);

private:
    /**
     * @brief The SDL texture managed by this class.
     *        If the texture is unmodified, this will reference the renderer's texture.
     *        If modified, it will be a separate texture.
     */
    Renderer::TextureVariant texture;

    /**
     * @brief Flag indicating if the texture is stored locally (modified).
     */
    bool textureStoredLocally = false;
};
} // namespace Nebulite::Core
#endif // NEBULITE_CORE_TEXTURE_HPP
