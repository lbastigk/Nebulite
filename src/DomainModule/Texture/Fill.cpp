#include "DomainModule/Texture/Fill.hpp"
#include "Core/Texture.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Error Fill::update() {
    // Nothing to do in update for fill
    return Constants::ErrorTable::NONE();
}

Constants::Error Fill::fill(int argc, char** argv) {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    // Get the SDL_Renderer
    if (Nebulite::global().getSdlRenderer() == nullptr) {
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

    // Lock the texture for pixel manipulation
    void* pixels;
    int pitch;
    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
        Nebulite::cerr() << "Failed to lock texture: " << SDL_GetError() << Nebulite::endl;
        return Constants::ErrorTable::TEXTURE::CRITICAL_TEXTURE_LOCK_FAILED();
    }

    // Fill the texture with the specified color
    auto* pixelData = static_cast<Uint32*>(pixels);
    int width, height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
    Uint32 const color = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), r, g, b);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pixelData[y * (pitch / 4) + x] = color;
        }
    }

    // Unlock the texture
    SDL_UnlockTexture(texture);

    Nebulite::cout() << "Texture filled with color: "
        << " R=" << static_cast<int>(r)
        << " G=" << static_cast<int>(g)
        << " B=" << static_cast<int>(b)
        << Nebulite::endl;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Texture
