//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Core/Renderer.hpp"
#include "DomainModule/Renderer/Console.hpp"
#include "DomainModule/Renderer/Input.hpp"
#include "Graphics/ImguiHelper.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Console::update() {
    // Toggling console mode
    static auto const toggleKey = Input::Key::keyboardDelta + "tab";
    if (moduleScope.get<int>(toggleKey).value_or(0) == 1) {
        consoleMode = !consoleMode;
        if (consoleMode) {
            SDL_StartTextInput(domain.getSdlWindow());
            SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST); // Flush all pending events
        } else {
            SDL_StopTextInput(domain.getSdlWindow());
        }
    }

    //------------------------------------------
    // Input handling

    if (consoleMode) {
        domain.skipUpdateNextFrame();
        static auto const accessToken = ScopeAccessor::Full();
        Graphics::ImguiHelper::DomainRenderingFlags flags;
        // TODO: set to false later on
        flags.showCloseButton = true; // Just for alignment testing, we activate it for now
        flags.windowAlignment = Graphics::ImguiHelper::DomainRenderingFlags::Alignment::BOTTOM;
        Graphics::ImguiHelper::renderDomain(Global::instance(), Global::capture(), Global::shareScopeBase(accessToken), "Console", flags);
    }

    //------------------------------------------
    // Return
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::GlobalSpace::Console
