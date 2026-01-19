#include "DomainModule/Texture/Rotation.hpp"
#include "Core/Texture.hpp"
#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Error Rotation::update() {
    // Nothing to do in update for rotation
    return Constants::ErrorTable::NONE();
}

Constants::Error Rotation::rotate(int const argc, char** argv) const {
    if (argc < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (argc > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Get the SDL_Renderer
    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::Texture
