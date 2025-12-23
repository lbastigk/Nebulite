#include "DomainModule/Texture/Rotation.hpp"
#include "Core/Texture.hpp"
#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Error Rotation::update() {
    // Nothing to do in update for rotation
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error Rotation::rotate(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Get the SDL_Renderer
    SDL_Renderer* renderer = Nebulite::global().getSdlRenderer();
    if (renderer == nullptr) {
        return Constants::ErrorTable::SDL::CRITICAL_SDL_RENDERER_INIT_FAILED();
    }

    // Get the texture to rotate
    SDL_Texture* texture = domain->getSDLTexture();
    if (texture == nullptr) {
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_NOT_FOUND();
    }

    // Get the texture's width and height
    int width, height;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_QUERY_FAILED();
    }

    // Create a new texture to hold the rotated result
    SDL_Texture* rotatedTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (rotatedTexture == nullptr) {
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_MODIFICATION_FAILED();
    }

    // Set the new texture as the render target
    if (SDL_SetRenderTarget(renderer, rotatedTexture) != 0) {
        SDL_DestroyTexture(rotatedTexture);
        return Constants::ErrorTable::SDL::CRITICAL_SDL_RENDERER_TARGET_FAILED();
    }

    // Clear the render target
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Transparent background
    SDL_RenderClear(renderer);

    // Rotate the texture and render it to the new texture
    double const angle = std::stod(argv[1]);
    SDL_Point const center = {width / 2, height / 2}; // Rotate around the center
    SDL_RendererFlip constexpr flip = SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, texture, nullptr, nullptr, angle, &center, flip);
    SDL_SetRenderTarget(renderer, nullptr);

    // Replace the original texture with the rotated texture
    domain->setInternalTexture(rotatedTexture);

    Nebulite::cout() << "Texture rotated by " << angle << " degrees." << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

std::string const Rotation::rotate_name = "rotate";
std::string const Rotation::rotate_desc = R"(Rotate the texture by a given angle

Usage: rotate <angle>
)";

} // namespace Nebulite::DomainModule::Texture
