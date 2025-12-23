#include "DomainModule/Texture/General.hpp"

#include "Constants/KeyNames.hpp"
#include "Core/Texture.hpp"
#include "Data/JSON.hpp"

namespace Nebulite::DomainModule::Texture {

Constants::Error General::update() {
    // No periodic updates needed for general functions
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error General::reloadTexture(int argc, char** argv) {
    // Check for correct number of arguments
    if (argc != 1) {
        // Only the command itself should be present
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Load new texture from the document
    auto const path = domain->getDoc()->get<std::string>(Constants::KeyNames::RenderObject::imageLocation, "");
    if (path.empty()) {
        return Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE(); // No valid path in document
    }
    domain->loadTextureFromFile(path);

    return Constants::ErrorTable::NONE(); // No error
}

std::string const General::reloadTexture_name = "reload-texture";
std::string const General::reloadTexture_desc = R"(Reload the texture from the document.

Usage: reload-texture
)";

} // namespace Nebulite::DomainModule::Texture