#include "DomainModule/Texture/General.hpp"

#include "Constants/KeyNames.hpp"
#include "Core/Texture.hpp"
#include "Data/Document/JSON.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Error General::update() {
    // No periodic updates needed for general functions
    return Constants::ErrorTable::NONE();
}

Constants::Error General::reloadTexture() {
    // Load new texture from the document
    auto const path = domain.getDoc()->get<std::string>(Constants::KeyNames::RenderObject::imageLocation, "");
    if (path.empty()) {
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE(); // No valid path in document
    }
    domain.loadTextureFromFile(path);

    return Constants::ErrorTable::NONE(); // No error
}

} // namespace Nebulite::DomainModule::Texture
