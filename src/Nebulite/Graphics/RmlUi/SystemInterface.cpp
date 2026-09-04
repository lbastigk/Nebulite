//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstdint>
#include <functional>

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/StringUtilities.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <utility>

// Nebulite
#include "Nebulite/Graphics/RmlUi/SystemInterface.hpp"
#include "Nebulite/Math/Vec2.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"
#include "Nebulite/Utility/TimeKeeper.hpp"

//------------------------------------------
namespace {
class Cursor {
    SDL_Cursor* data = nullptr;
    Nebulite::Utility::TimeKeeper usageTracker;

public:
    explicit Cursor(SDL_SystemCursor const& id, auto&& condition) : data(SDL_CreateSystemCursor(id)), enableCondition(condition) {
        usageTracker.start();
        usageTracker.update();
    }

    [[nodiscard]] SDL_Cursor* get() const {
        return data;
    }

    std::uint64_t dt() {
        return usageTracker.dtProjected();
    }

    void update(Rml::String const& currentCursorName) {
        if (enableCondition(currentCursorName)) {
            usageTracker.update();
        }
    }

    void forceUpdate() {
        usageTracker.update();
    }

    std::function<bool(Rml::String const&)> enableCondition;
};

auto& availableCursors() {
    static std::array cursors = {
        Cursor{SDL_SYSTEM_CURSOR_MOVE, [](Rml::String const& cursorName){ return cursorName == "move" || Rml::StringUtilities::StartsWith(cursorName, "rmlui-scroll"); }},
        Cursor{SDL_SYSTEM_CURSOR_POINTER, [](Rml::String const& cursorName){ return cursorName == "pointer"; }},
        Cursor{SDL_SYSTEM_CURSOR_NWSE_RESIZE, [](Rml::String const& cursorName){ return cursorName == "resize"; }},
        Cursor{SDL_SYSTEM_CURSOR_CROSSHAIR, [](Rml::String const& cursorName){ return cursorName == "cross"; }},
        Cursor{SDL_SYSTEM_CURSOR_TEXT, [](Rml::String const& cursorName){ return cursorName == "text"; }},
        Cursor{SDL_SYSTEM_CURSOR_NOT_ALLOWED, [](Rml::String const& cursorName){ return cursorName == "unavailable"; }},
        Cursor{SDL_SYSTEM_CURSOR_DEFAULT, [](Rml::String const& cursorName){ return cursorName.empty() || cursorName == "arrow"; }},
    };
    return cursors;
}

} // namespace

//------------------------------------------
namespace Nebulite::Graphics {

RmlSystemInterface::RmlSystemInterface(SDL_Window* w, Utility::Io::Capture& c) : capture(c) {
    SetWindow(w);
}

void RmlSystemInterface::update(int const mousePositionX, int const mousePositionY){
    // We order the candidates by priority and pick the first one that has been updated within the hold duration
    static auto constexpr holdDuration = 200; // ms
    SDL_Cursor* newCursor = nullptr;
    for (auto& candidate : availableCursors()) {
        if (candidate.dt() <= holdDuration) {
            newCursor = candidate.get();
            break;
        }
    }
    // Only reset cursor if mouse has moved
    currentMousePosition = Position{
        .x=mousePositionX,
        .y=mousePositionY
    };
    if (!newCursor && currentMousePosition != lastMousePosition) {
        availableCursors().back().forceUpdate();
    }
    lastMousePosition = currentMousePosition;

    // Only update if cursor changed
    static SDL_Cursor const* lastCursor = nullptr;
    if (newCursor && newCursor != lastCursor) {
        lastCursor = newCursor;
        SDL_SetCursor(newCursor);
    }
}

void RmlSystemInterface::disableLogging() {
    logEnabled = false;
}

void RmlSystemInterface::enableLogging() {
    logEnabled = true;
}

bool RmlSystemInterface::LogMessage(Rml::Log::Type const type, Rml::String const& message) {
    if (!logEnabled) return true;

    // Redirect RmlUi log messages to our capture system
    switch (type) {
    // Log
    case Rml::Log::LT_ALWAYS:
    case Rml::Log::LT_INFO:
    case Rml::Log::LT_DEBUG:
        capture.log.println(message);
        break;
    // Warning
    case Rml::Log::LT_WARNING:
        capture.warning.println(message);
        break;
    // Error
    case Rml::Log::LT_ERROR:
    case Rml::Log::LT_ASSERT:
        capture.error.println(message);
        break;
    // else
    case Rml::Log::LT_MAX:
    default:
        std::unreachable();
    }
    return true; // Continue execution
}

void RmlSystemInterface::SetMouseCursor(Rml::String const& cursorName) {
    // Update all TON-like-timekeepers
    for (auto& candidate : availableCursors()) {
        candidate.update(cursorName);
    }
}

} // namespace Nebulite::Graphics
