#include "DomainModule/Texture/TXDM_Rotation.hpp"

#include "Core/GlobalSpace.hpp"
#include "Core/Texture.hpp"

namespace Nebulite::DomainModule::Texture {

Nebulite::Constants::Error Nebulite::DomainModule::Texture::Rotation::update() {
    // Nothing to do in update for rotation
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::Texture::Rotation::rotate(int argc,  char* argv[]) {
    if (argc < 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Get the SDL_Renderer
    SDL_Renderer* renderer = domain->getGlobalSpace()->getSdlRenderer();
    if (renderer == nullptr) {
        return Nebulite::Constants::ErrorTable::SDL::CRITICAL_SDL_RENDERER_INIT_FAILED();
    }

    // Get the texture to rotate
    SDL_Texture* texture = domain->getSDLTexture();
    if (texture == nullptr) {
        return Nebulite::Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_NOT_FOUND();
    }

    // Get the texture's width and height
    int width, height;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        return Nebulite::Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_QUERY_FAILED();
    }

    // Create a new texture to hold the rotated result
    SDL_Texture* rotatedTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (rotatedTexture == nullptr) {
        return Nebulite::Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_MODIFICATION_FAILED();
    }

    // Set the new texture as the render target
    if (SDL_SetRenderTarget(renderer, rotatedTexture) != 0) {
        SDL_DestroyTexture(rotatedTexture);
        return Nebulite::Constants::ErrorTable::SDL::CRITICAL_SDL_RENDERER_TARGET_FAILED();
    }

    // Clear the render target
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Transparent background
    SDL_RenderClear(renderer);

    // Rotate the texture and render it to the new texture
    float angle = std::stof(argv[1]);
    SDL_Point center = {width / 2, height / 2}; // Rotate around the center
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, texture, nullptr, nullptr, angle, &center, flip);
    SDL_SetRenderTarget(renderer, nullptr);

    // Replace the original texture with the rotated texture
    domain->setInternalTexture(rotatedTexture);

    capture->cout << "Texture rotated by " << angle << " degrees." << capture->endl;
    return Nebulite::Constants::ErrorTable::NONE();
}
const std::string Rotation::rotate_name = "rotate";
const std::string Rotation::rotate_desc = R"(Rotate the texture by a given angle

Usage: rotate <angle>
)";

}  // namespace Nebulite::DomainModule::Texture