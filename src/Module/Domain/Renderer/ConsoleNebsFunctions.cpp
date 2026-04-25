//------------------------------------------
// Includes

// Nebulite
#include "Module/Domain/Renderer/Console.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event Console::consoleOpen() {
    consoleMode = true;
    return Constants::Event::Success;
}

Constants::Event Console::consoleClose() {
    consoleMode = false;
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Renderer
