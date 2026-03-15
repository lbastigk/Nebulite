//------------------------------------------
// Includes

// Nebulite
#include "DomainModule/Renderer/Console.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Console::consoleOpen() {
    consoleMode = true;
    return Constants::ErrorTable::NONE();
}

Constants::Error Console::consoleClose() {
    consoleMode = false;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Renderer
