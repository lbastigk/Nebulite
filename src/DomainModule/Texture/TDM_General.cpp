#include "DomainModule/Texture/TDM_General.hpp"

#include "Core/Texture.hpp"
#include "Utility/JSON.hpp"

void Nebulite::DomainModule::Texture::General::update() {
    // No periodic updates needed for general functions
}

Nebulite::Constants::ERROR_TYPE Nebulite::DomainModule::Texture::General::reloadTexture(int argc, char* argv[]) {
    // Check for correct number of arguments
    if (argc != 1) { // Only the command itself should be present
        return Nebulite::Constants::ERROR_TYPE::TOO_MANY_ARGS;
    }

    // Load new texture from the document
    std::string path = domain->getDoc()->get<std::string>(Nebulite::Constants::keyName.renderObject.imageLocation.c_str(), "");
    if(path.empty()) {
        return Nebulite::Constants::ERROR_TYPE::CRITICAL_INVALID_FILE; // No valid path in document
    }
    domain->loadTextureFromFile(path);

    return Nebulite::Constants::NONE; // No error
}