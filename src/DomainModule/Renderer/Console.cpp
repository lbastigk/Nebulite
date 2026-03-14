//------------------------------------------
// Includes

// External
#include <imgui.h>

// Nebulite
#include "Nebulite.hpp"
#include "Constants/KeyNames.hpp"
#include "Core/Renderer.hpp"
#include "DomainModule/Renderer/Console.hpp"
#include "DomainModule/Renderer/Input.hpp"
#include "Graphics/ImguiHelper.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

Constants::Error Console::update() {
    // Execute autoType commands
    processAutotypeQueue();

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

void Console::processAutotypeQueue() {
    autoType.waitTimer.update();
    auto const dt_ms = autoType.waitTimer.get_dt_ms();
    if (autoType.waitTimeRemaining > 0) {
        if (dt_ms >= autoType.waitTimeRemaining) {
            autoType.waitTimeRemaining = 0;
        } else {
            autoType.waitTimeRemaining -= dt_ms;
        }
    } else {
        while (!autoType.activeQueue.empty()) {
            if (autoType.waitTimeRemaining > 0) {
                break; // Wait time set by a command, pause execution of further commands until next update
            }
            switch (auto const& [type, text] = autoType.activeQueue.front(); type) {
            case AutoType::Command::Type::TEXT:
                // TODO: Insert text into Imgui input buffer
                break;
            case AutoType::Command::Type::ENTER:
                // TODO: Insert submit into Imgui input buffer
                break;
            case AutoType::Command::Type::CLOSE:
                consoleMode = false;
                SDL_StopTextInput(domain.getSdlWindow());
                break;
            case AutoType::Command::Type::WAIT:
                try {
                    autoType.waitTimeRemaining = std::stoul(text);
                } catch (std::exception const&) {
                    domain.capture.error.println("Invalid wait time in autoType command: ", text);
                }
                break;
            case AutoType::Command::Type::HISTORY_UP:
                // TODO: Inject events into the imgui pipeline
                break;
            case AutoType::Command::Type::HISTORY_DOWN:
                // TODO: Inject events into the imgui pipeline
                break;
            default:
                std::unreachable();
            }
            autoType.activeQueue.pop();
        }
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace::Console
