//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Core/Renderer.hpp"
#include "Module/Domain/Renderer/Console.hpp"
#include "Graphics/ImguiHelper.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Event Console::updateHook() {
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

        Interaction::Context ctx = {Global::instance(), Global::instance(), Global::instance()};
        Interaction::ContextScope ctxScope = {Global::shareScope(accessToken), Global::shareScope(accessToken), Global::shareScope(accessToken)};
        Graphics::ImguiHelper::renderDomain(ctx, ctxScope, Global::capture(), "Console", flags);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::GlobalSpace::Console
