#include "DomainModule/Texture/Rotation.hpp"
#include "Core/Texture.hpp"
#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Event Rotation::update() {
    // Nothing to do in update for rotation
    return Constants::Event::Success;
}

Constants::Event Rotation::rotate(int const argc, char** argv) const {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (argc > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    // Get the SDL_Renderer
    SDL_Renderer* renderer = Global::instance().getSdlRenderer();
    if (renderer == nullptr) {
        return Constants::StandardCapture::Error::SDL::initFailed(domain.capture);
    }

    // Get the texture to rotate
    SDL_Texture* texture = domain.getSDLTexture();
    if (texture == nullptr) {
        return Constants::StandardCapture::Error::Texture::notFound(domain.capture);
    }

    // Get the texture's width and height
    float width, height;
    if (SDL_GetTextureSize(texture, &width, &height) != 0) {
        return Constants::StandardCapture::Error::Texture::queryFailed(domain.capture);
    }

    // Create a new texture to hold the rotated result
    SDL_Texture* rotatedTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, static_cast<int>(width), static_cast<int>(height));
    if (rotatedTexture == nullptr) {
        return Constants::StandardCapture::Error::Texture::modificationFailed(domain.capture);
    }

    // Set the new texture as the render target
    if (SDL_SetRenderTarget(renderer, rotatedTexture) != 0) {
        SDL_DestroyTexture(rotatedTexture);
        return Constants::StandardCapture::Error::SDL::settingTargetFailed(domain.capture);
    }

    // Clear the render target
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Transparent background
    SDL_RenderClear(renderer);

    // Rotate the texture and render it to the new texture
    double const angle = std::stod(argv[1]);
    SDL_FPoint const centerFloat = {width / 2.0f, height / 2.0f};
    SDL_FlipMode constexpr flip = SDL_FLIP_NONE;
    SDL_RenderTextureRotated(renderer, texture, nullptr, nullptr, angle, &centerFloat, flip);
    SDL_SetRenderTarget(renderer, nullptr);

    // Replace the original texture with the rotated texture
    domain.setInternalTexture(rotatedTexture);
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::Texture
