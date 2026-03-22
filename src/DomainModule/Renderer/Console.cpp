//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Core/Renderer.hpp"
#include "DomainModule/Renderer/Console.hpp"
#include "Graphics/ImguiHelper.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Event Console::update() {
    static auto const toggleKey = Input::Key::keyboardDelta + "tab";
    if (moduleScope.get<int>(toggleKey).value_or(0) == 1) {
        consoleMode = !consoleMode;
    }
    if (consoleMode) {
        domain.skipUpdateNextFrame();
        static auto const accessToken = ScopeAccessor::Full();
        Graphics::ImguiHelper::DomainRenderingFlags flags;
        flags.showCloseButton = false;
        flags.windowAlignment = Graphics::ImguiHelper::DomainRenderingFlags::Alignment::BOTTOM;
        Graphics::ImguiHelper::renderDomain(Global::instance(), Global::capture(), Global::shareScope(accessToken), "Console", flags);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::GlobalSpace::Console
