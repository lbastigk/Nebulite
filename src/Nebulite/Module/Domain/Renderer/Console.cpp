//------------------------------------------
// Includes

// Standard library
#include <optional>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Core/GlobalSpace.hpp" // NOLINT(misc-include-cleaner)
#include "Nebulite/Core/Renderer.hpp"
#include "Nebulite/Graphics/ImguiHelper.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"
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
        Graphics::ImguiHelper::DomainRenderingFlags const flags{
            .showCloseButton = false,
            .windowPos = std::nullopt,
            .windowSize = std::nullopt,
            .windowAlignment = Graphics::ImguiHelper::DomainRenderingFlags::Alignment::bottom,
        };

        // Set context/scope
        auto& globalDomain = static_cast<Interaction::Execution::Domain&>(Global::instance());
        static auto const accessToken = ScopeAccessor::Full();
        auto& globalScope = Global::shareScope(accessToken);
        Interaction::Context ctx = {globalDomain, globalDomain, globalDomain};
        Interaction::ContextScope ctxScope = {globalScope, globalScope, globalScope};

        // Render
        Graphics::ImguiHelper::renderDomain(ctx, ctxScope, Global::capture(), "Console", flags);
    }
    return Constants::Event::success;
}

Constants::Event Console::consoleOpen() {
    consoleMode = true;
    return Constants::Event::success;
}

Constants::Event Console::consoleClose() {
    consoleMode = false;
    return Constants::Event::success;
}

Console::Console(ConstructorParams const& params) : DomainModule(params) {
    bindCategory(consoleName, consoleDesc);
    bindFunction(&Console::consoleOpen, consoleOpenName, consoleOpenDesc);
    bindFunction(&Console::consoleClose, consoleCloseName, consoleCloseDesc);
}

} // namespace Nebulite::Module::Domain::Renderer
