#include "DomainModule/Texture/Fill.hpp"
#include "Core/Texture.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Error Fill::update() {
    // Nothing to do in update for fill
    return Constants::ErrorTable::NONE();
}

Constants::Error Fill::fill(int const argc, char** argv) const {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    // Get the SDL_Renderer
    SDL_Renderer* renderer = Global::instance().getSdlRenderer();
    if (renderer == nullptr) {
        return Constants::ErrorTable::SDL::CRITICAL_SDL_RENDERER_INIT_FAILED();
    }

    // Get the texture to fill
    SDL_Texture* texture = domain.getSDLTexture();
    if (texture == nullptr) {
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_NOT_FOUND();
    }

    // Parse color arguments
    Uint8 r = 0, g = 0, b = 0;
    if (argc == 2) {
        if (std::string const color = argv[1]; color == "red") {
            r = 255;
        } else if (color == "green") {
            g = 255;
        } else if (color == "blue") {
            b = 255;
        } else {
            return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_COLOR_UNSUPPORTED();
        }
    } else if (argc == 4) {
        r = static_cast<Uint8>(std::stoi(argv[1]));
        g = static_cast<Uint8>(std::stoi(argv[2]));
        b = static_cast<Uint8>(std::stoi(argv[3]));
    } else {
        return Constants::ErrorTable::FUNCTIONAL::CRITICAL_INVALID_ARGC_ARGV_PARSING();
    }

    // Bind texture as render target, fill with draw color, restore previous target.
    SDL_Texture* prevTarget = SDL_GetRenderTarget(renderer); // may be nullptr
    if (SDL_SetRenderTarget(renderer, texture) != 0) {
        domain.capture().error.println("Failed to set render target: ", SDL_GetError());
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_LOCK_FAILED();
    }

    if (SDL_SetRenderDrawColor(renderer, r, g, b, 255) != 0) {
        domain.capture().error.println("Failed to set draw color: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, prevTarget);
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_LOCK_FAILED();
    }

    if (SDL_RenderClear(renderer) != 0) {
        domain.capture().error.println("Failed to clear (fill) texture: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, prevTarget);
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_LOCK_FAILED();
    }

    // restore previous render target
    SDL_SetRenderTarget(renderer, prevTarget);

    // Finish
    domain.capture().error.println("Texture filled with color:"
        , " R=", static_cast<int>(r)
        , " G=", static_cast<int>(g)
        , " B=", static_cast<int>(b));
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Texture
