//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp" // NOLINT
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Graphics/ImguiHelper.hpp"
#include "Nebulite/Module/Domain/Renderer/Console.hpp"
#include "Nebulite/Module/Domain/Renderer/Input.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event Console::updateHook() {
    static auto const toggleKey = Input::Key::keyboardDelta.addMember("`");
    if (moduleScope.get<int>(toggleKey).value_or(0) == 1) {
        consoleMode = !consoleMode;
    }
    if (consoleMode) {
        domain.skipUpdateNextFrame();

        // Set console flags
        Graphics::ImguiHelper::DomainRenderingFlags flags;
        flags.showCloseButton = false;
        flags.windowAlignment = Graphics::ImguiHelper::DomainRenderingFlags::Alignment::BOTTOM;

        // Set context/scope
        Interaction::Execution::Domain& global = Global::instance();
        static auto const accessToken = ScopeAccessor::Full();
        auto& globalScope = Global::shareScope(accessToken);
        Interaction::Context ctx = {global, global, global};
        Interaction::ContextScope ctxScope = {globalScope, globalScope, globalScope};

        // Render
        Graphics::ImguiHelper::renderDomain(ctx, ctxScope, Global::capture(), "Console", flags);
    }
    return Constants::Event::Success;
}

Constants::Event Console::consoleOpen() {
    consoleMode = true;
    return Constants::Event::Success;
}

Constants::Event Console::consoleClose() {
    consoleMode = false;
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Renderer
