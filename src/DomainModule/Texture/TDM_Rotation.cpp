#include "DomainModule/Texture/TDM_Rotation.hpp"

#include "Core/GlobalSpace.hpp"
#include "Core/Texture.hpp"

void Nebulite::DomainModule::Texture::Rotation::update() {
    // Nothing to do in update for rotation
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::Texture::Rotation::rotate(int argc, char* argv[]) {
    if (argc < 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_FEW_ARGS;
    }
    if (argc > 2) {
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    // Get the SDL_Renderer
    SDL_Renderer* renderer = domain->getGlobalSpace()->getSDLRenderer();
    if (renderer == nullptr) {
        domain->getGlobalSpace()->parseStr(
            "Nebulite::DomainModule::Texture::Rotation::rotate error No renderer available"
        );
        return Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR;
    }

    // Get the texture to rotate
    SDL_Texture* texture = domain->getSDLTexture();
    if (texture == nullptr) {
        domain->getGlobalSpace()->parseStr(
            "Nebulite::DomainModule::Texture::Rotation::rotate error No texture to rotate"
        );
        return Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR;
    }

    // Get the texture's width and height
    int width, height;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        domain->getGlobalSpace()->parseStr(
            "Nebulite::DomainModule::Texture::Rotation::rotate error Failed to query texture"
        );
        return Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR;
    }

    // Create a new texture to hold the rotated result
    SDL_Texture* rotatedTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (rotatedTexture == nullptr) {
        domain->getGlobalSpace()->parseStr(
            "Nebulite::DomainModule::Texture::Rotation::rotate error Failed to create new texture"
        );
        return Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR;
    }

    // Set the new texture as the render target
    if (SDL_SetRenderTarget(renderer, rotatedTexture) != 0) {
        SDL_DestroyTexture(rotatedTexture);
        domain->getGlobalSpace()->parseStr(
            "Nebulite::DomainModule::Texture::Rotation::rotate error Failed to set render target, error: " + std::string(SDL_GetError())
        );
        return Nebulite::Constants::ERROR_TYPE::CUSTOM_ERROR;
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

    std::cout << "Texture rotated by " << angle << " degrees." << std::endl;
    return Nebulite::Constants::ERROR_TYPE::NONE;
}