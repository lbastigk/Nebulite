#include "DomainModule/Texture/Fill.hpp"
#include "Core/Texture.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Event Fill::updateHook() {
    // Nothing to do in update for fill
    return Constants::Event::Success;
}

Constants::Event Fill::fill(int const argc, char** argv) const {
    if (argc < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }

    // Get the SDL_Renderer
    SDL_Renderer* renderer = Global::instance().getSdlRenderer();
    if (renderer == nullptr) {
        return Constants::StandardCapture::Error::SDL::initFailed(domain.capture);
    }

    // Get the texture to fill
    SDL_Texture* texture = domain.getSDLTexture();
    if (texture == nullptr) {
        return Constants::StandardCapture::Error::Texture::notFound(domain.capture);
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
            return Constants::StandardCapture::Error::Texture::colorUnsupported(domain.capture);
        }
    } else if (argc == 4) {
        r = static_cast<Uint8>(std::stoi(argv[1]));
        g = static_cast<Uint8>(std::stoi(argv[2]));
        b = static_cast<Uint8>(std::stoi(argv[3]));
    } else {
        return Constants::StandardCapture::Warning::Functional::invalidArgcArgvParsing(domain.capture);
    }

    // Bind texture as render target, fill with draw color, restore previous target.
    SDL_Texture* prevTarget = SDL_GetRenderTarget(renderer); // may be nullptr
    if (SDL_SetRenderTarget(renderer, texture) != 0) {
        domain.capture.error.println("Failed to set render target: ", SDL_GetError());
        return Constants::StandardCapture::Error::Texture::lockFailed(domain.capture);
    }

    if (SDL_SetRenderDrawColor(renderer, r, g, b, 255) != 0) {
        domain.capture.error.println("Failed to set draw color: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, prevTarget);
        return Constants::StandardCapture::Error::Texture::lockFailed(domain.capture);
    }

    if (SDL_RenderClear(renderer) != 0) {
        domain.capture.error.println("Failed to clear (fill) texture: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, prevTarget);
        return Constants::StandardCapture::Error::Texture::lockFailed(domain.capture);
    }

    // restore previous render target
    SDL_SetRenderTarget(renderer, prevTarget);

    // Finish
    domain.capture.error.println("Texture filled with color:"
        , " R=", static_cast<int>(r)
        , " G=", static_cast<int>(g)
        , " B=", static_cast<int>(b));
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::Texture
