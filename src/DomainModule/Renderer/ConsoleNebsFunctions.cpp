//------------------------------------------
// Includes

// Nebulite
#include "DomainModule/Renderer/Console.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Event Console::consoleOpen() {
    consoleMode = true;
    return Constants::Event::Success;
}

Constants::Event Console::consoleClose() {
    consoleMode = false;
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::Renderer
