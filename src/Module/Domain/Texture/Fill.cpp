//------------------------------------------
// Includes

// Standard library
#include <string>

// External
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Core/Texture.hpp"
#include "Module/Domain/Texture/Fill.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Texture {

Constants::Event Fill::updateHook() {
    // Nothing to do in update for fill
    return Constants::Event::Success;
}

Constants::Event Fill::fill(int const argc, char const** argv) const {
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
    Uint8 r = 0;
    Uint8 g = 0;
    Uint8 b = 0;
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
    if (!SDL_SetRenderTarget(renderer, texture)) {
        domain.capture.error.println("Failed to set render target: ", SDL_GetError());
        return Constants::StandardCapture::Error::Texture::lockFailed(domain.capture);
    }

    if (!SDL_SetRenderDrawColor(renderer, r, g, b, 255)) {
        domain.capture.error.println("Failed to set draw color: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, prevTarget);
        return Constants::StandardCapture::Error::Texture::lockFailed(domain.capture);
    }

    if (!SDL_RenderClear(renderer)) {
        domain.capture.error.println("Failed to clear (fill) texture: ", SDL_GetError());
        SDL_SetRenderTarget(renderer, prevTarget);
        return Constants::StandardCapture::Error::Texture::lockFailed(domain.capture);
    }

    // restore previous render target
    SDL_SetRenderTarget(renderer, prevTarget);

    // Finish
    domain.capture.log.println(
        "Texture filled with color:"
        , " R=", static_cast<int>(r)
        , " G=", static_cast<int>(g)
        , " B=", static_cast<int>(b)
    );
    return Constants::Event::Success;
}

Constants::Event Fill::averageColor() const {
    SDL_Texture* texture = domain.getSDLTexture();
    if (!texture) {
        return Constants::StandardCapture::Error::Texture::notFound(domain.capture);
    }

    SDL_Renderer* renderer = Global::instance().getSdlRenderer();
    if (!renderer) {
        return Constants::StandardCapture::Error::Renderer::notInitialized(domain.capture);
    }

    // Set target to texture
    SDL_Texture* prevTarget = SDL_GetRenderTarget(renderer); // may be nullptr
    SDL_SetRenderTarget(renderer, texture);

    // Get surface
    float fw, fh;
    if (!SDL_GetTextureSize(texture, &fw, &fh)) {
        return Constants::StandardCapture::Error::Texture::queryFailed(domain.capture);
    }
    int const w = static_cast<int>(fw);
    int const h = static_cast<int>(fh);
    SDL_Rect const rect{0, 0, w, h};
    auto* const surf = SDL_RenderReadPixels(renderer, &rect);
    if (!surf) {
        return Constants::StandardCapture::Error::Texture::queryFailed(domain.capture);
    }

    // Set target back to previous
    SDL_SetRenderTarget(renderer, prevTarget);

    // Calculate average
    Uint64 r = 0;
    Uint64 g = 0;
    Uint64 b = 0;
    Uint64 a = 0;
    for (auto [x, y] : std::views::cartesian_product(std::views::iota(0, w), std::views::iota(0, h))) {
        Uint8 pr, pg, pb, pa;
        if (!SDL_ReadSurfacePixel(surf, x, y, &pr, &pg, &pb, &pa)) {
            return Constants::StandardCapture::Error::Texture::queryFailed(domain.capture);
        }
        r += pr;
        g += pg;
        b += pb;
        a += pa;
    }

    SDL_DestroySurface(surf);

    // Print average
    auto const count = w * h;
    domain.capture.log.println(
        "Average color of texture:"
        , " R=", static_cast<double>(r) / static_cast<double>(count)
        , " G=", static_cast<double>(g) / static_cast<double>(count)
        , " B=", static_cast<double>(b) / static_cast<double>(count)
        , " A=", static_cast<double>(a) / static_cast<double>(count)
    );

    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Texture
