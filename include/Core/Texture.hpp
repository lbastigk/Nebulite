/**
 * @file Texture.hpp
 * 
 * Proposed new Domain for Nebulite to bundle all texture related
 * functionality.
 */

#pragma once

//------------------------------------------
// Includes

// External
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// Nebulite
#include "Core/Renderer.hpp"
#include "Interaction/Execution/Domain.hpp"

//------------------------------------------
namespace Nebulite {
namespace Core {
class Texture : public Nebulite::Interaction::Execution::Domain<Texture>{
public:
    /**
     * @brief Constructs a new Texture domain.
     * 
     * @param doc Pointer to the JSON document for storing texture properties.
     * @param renderer Pointer to the SDL_Renderer for texture operations.
     * @param texture Optional pointer to an existing SDL_Texture.
     */
    Texture(Nebulite::Utility::JSON* doc, SDL_Renderer* renderer, SDL_Texture* texture = nullptr);

    /**
     * @brief Updates the texture.
     */
    void update() override;
private:
    /**
     * @brief Reference to the renderer for texture operations.
     */
    SDL_Renderer* renderer;

    /**
     * @brief The SDL texture managed by this class.
     * If the texture is unmodified, this will reference the renderer's texture.
     * If modified, it will be a separate texture.
     */
    SDL_Texture* texture;

    bool textureModified = false; // Flag to indicate if the texture has been modified

    /**
     * @brief Makes a copy of the texture currently managed by this class.
     */
    void copyTexture();
};
}   // namespace Core
}   // namespace Nebulite