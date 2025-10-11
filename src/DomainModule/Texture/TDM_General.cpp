#include "DomainModule/Texture/TDM_General.hpp"

#include "Core/Texture.hpp"
#include "Utility/JSON.hpp"

namespace Nebulite::DomainModule::Texture {

Nebulite::Constants::Error Nebulite::DomainModule::Texture::General::update() {
    // No periodic updates needed for general functions
    return Nebulite::Constants::ErrorTable::NONE();
}

Nebulite::Constants::Error Nebulite::DomainModule::Texture::General::reloadTexture(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc != 1) { // Only the command itself should be present
        return Nebulite::Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Load new texture from the document
    std::string path = domain->getDoc()->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str(), "");
    if(path.empty()) {
        return Nebulite::Constants::ErrorTable::FILE::CRITICAL_INVALID_FILE(); // No valid path in document
    }
    domain->loadTextureFromFile(path);

    return Nebulite::Constants::ErrorTable::NONE(); // No error
}
const std::string General::reloadTexture_name = "reload-texture";
const std::string General::reloadTexture_desc = R"(Reload the texture from the document.

Usage: reload-texture
)";

} // namespace Nebulite::DomainModule::Texture   

