//------------------------------------------
// Includes

// External
#include <RmlUi/Core/StringUtilities.h>

// Nebulite
#include "Graphics/RmlSystemInterface.hpp"

//------------------------------------------
namespace Nebulite::Graphics {

RmlSystemInterface::RmlSystemInterface(SDL_Window* w, Utility::IO::Capture& c) : SystemInterface_SDL(w), capture(c) {}

void RmlSystemInterface::update(){
    // We order the candidates by priority and pick the first one that has been updated within the hold duration
    static auto constexpr holdDuration = 200; // ms
    static std::array priority = {
        &availableCursors.move,
        &availableCursors.pointer,
        &availableCursors.resize,
        &availableCursors.cross,
        &availableCursors.text,
        &availableCursors.unavailable,
        &availableCursors.general // Should always be last since it's the default cursor
    };
    SDL_Cursor* newCursor = nullptr;
    for (auto const& candidate : priority) {
        if (candidate->dt() <= holdDuration) {
            newCursor = candidate->get();
            break;
        }
    }
    if (!newCursor) { // No cursor active, fall back to lowest priority
        newCursor = priority.back()->get();
    }

    // Only update if cursor changed
    static SDL_Cursor* lastCursor = nullptr;
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

void RmlSystemInterface::SetMouseCursor(const Rml::String& cursor_name) {
    // Update all TON-like-timekeepers
    auto cursorUpdater = [] (Cursor& candidate, auto&& condition) {
        if (condition()) {candidate.update();}
    };
    cursorUpdater(availableCursors.general, [&]{ return cursor_name.empty() || cursor_name == "arrow"; });
    cursorUpdater(availableCursors.move, [&]{ return cursor_name == "move" || Rml::StringUtilities::StartsWith(cursor_name, "rmlui-scroll"); });
    cursorUpdater(availableCursors.pointer, [&]{ return cursor_name == "pointer"; });
    cursorUpdater(availableCursors.resize, [&]{ return cursor_name == "resize"; });
    cursorUpdater(availableCursors.cross, [&]{ return cursor_name == "cross"; });
    cursorUpdater(availableCursors.text, [&]{ return cursor_name == "text"; });
    cursorUpdater(availableCursors.unavailable, [&]{ return cursor_name == "unavailable"; });
}

} // namespace Nebulite::Graphics
