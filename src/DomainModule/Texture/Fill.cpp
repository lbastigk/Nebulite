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

    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::Texture
