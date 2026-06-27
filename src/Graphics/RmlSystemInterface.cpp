//------------------------------------------
// Includes

// Standard library
#include <utility>

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi_Platform_SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

// Nebulite
#include "Graphics/RmlSystemInterface.hpp"
#include "Utility/IO/Capture.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

RmlSystemInterface::RmlSystemInterface(SDL_Window* w, Utility::IO::Capture& c) : SystemInterface_SDL(w), capture(c) {}

void RmlSystemInterface::update(int const mousePositionX, int const mousePositionY){
    // We order the candidates by priority and pick the first one that has been updated within the hold duration
    static auto constexpr holdDuration = 200; // ms
    SDL_Cursor* newCursor = nullptr;
    for (auto& candidate : availableCursors) {
        if (candidate.dt() <= holdDuration) {
            newCursor = candidate.get();
            break;
        }
    }
    // Only reset cursor if mouse has moved
    currentMousePosition = {.x=mousePositionX, .y=mousePositionY};
    if (!newCursor && currentMousePosition != lastMousePosition) {
        availableCursors.back().forceUpdate();
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

void RmlSystemInterface::SetMouseCursor(Rml::String const& cursor_name) {
    // Update all TON-like-timekeepers
    for (auto& candidate : availableCursors) {
        candidate.update(cursor_name);
    }
}

} // namespace Nebulite::Graphics
