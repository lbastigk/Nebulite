//------------------------------------------
// Includes

// Nebulite
#include "Core/Texture.hpp"
#include "DomainModule/Texture/General.hpp"
#include "Graphics/Drawcall.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Texture {

Constants::Event General::updateHook() {
    return Constants::Event::Success;
}

Constants::Event General::reloadTexture() const {
    // Load new texture from the document
    auto const path = moduleScope.get<std::string>(Graphics::Drawcall::Key::SpriteSpecific::imageLocation).value_or("");
    if (path.empty()) {
        return Constants::StandardCapture::Error::File::invalidFile(domain.capture);// No valid path in document
    }
    domain.loadTextureFromFile(path);
    return Constants::Event::Success; // No error
}

} // namespace Nebulite::DomainModule::Texture
