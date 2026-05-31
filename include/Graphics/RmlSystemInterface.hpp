#ifndef GRAPHICS_RMLSYSTEMINTERFACE_HPP
#define GRAPHICS_RMLSYSTEMINTERFACE_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi_Platform_SDL.h>

// Nebulite
#include "Utility/IO/Capture.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
namespace Nebulite::Graphics {
/**
 * @brief Custom interface to integrate RmlUis SDL SystemInterface with Nebulites capture system
 */
class RmlSystemInterface final : public SystemInterface_SDL {
public:
    RmlSystemInterface(SDL_Window* w, Utility::IO::Capture& c);

    void update(int const& mousePositionX, int const& mousePositionY);

    void disableLogging();

    void enableLogging();

    /**
     * @brief Forward Rml Log messages to the provided capture
     * @param type The type of message
     * @param message The message to print
     * @return True to continue execution, false to break into the debugger.
     */
    bool LogMessage(Rml::Log::Type type, Rml::String const& message) override ;

    /**
     * @brief Sets the mouse cursor to the correct type, trying to minimize any rapid cursor changes
     * @param cursor_name The name of the cursor
     */
    void SetMouseCursor(Rml::String const& cursor_name) override ;

private:
    Utility::IO::Capture& capture;
    bool logEnabled = true;

    class Cursor {
        SDL_Cursor* data = nullptr;
        Utility::TimeKeeper usageTracker;

    public:
        explicit Cursor(SDL_SystemCursor const& id, auto&& condition) : data(SDL_CreateSystemCursor(id)), enableCondition(condition) {
            usageTracker.start();
            usageTracker.update();
        }

        [[nodiscard]] SDL_Cursor* get() const {
            return data;
        }

        uint64_t dt() {
            return usageTracker.projected_dt();
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

    std::array<Cursor, 7> availableCursors = {
        Cursor{SDL_SYSTEM_CURSOR_MOVE, [](Rml::String const& cursorName){ return cursorName == "move" || Rml::StringUtilities::StartsWith(cursorName, "rmlui-scroll"); }},
        Cursor{SDL_SYSTEM_CURSOR_POINTER, [](Rml::String const& cursorName){ return cursorName == "pointer"; }},
        Cursor{SDL_SYSTEM_CURSOR_NWSE_RESIZE, [](Rml::String const& cursorName){ return cursorName == "resize"; }},
        Cursor{SDL_SYSTEM_CURSOR_CROSSHAIR, [](Rml::String const& cursorName){ return cursorName == "cross"; }},
        Cursor{SDL_SYSTEM_CURSOR_TEXT, [](Rml::String const& cursorName){ return cursorName == "text"; }},
        Cursor{SDL_SYSTEM_CURSOR_NOT_ALLOWED, [](Rml::String const& cursorName){ return cursorName == "unavailable"; }},
        Cursor{SDL_SYSTEM_CURSOR_DEFAULT, [](Rml::String const& cursorName){ return cursorName.empty() || cursorName == "arrow"; }}
    };

    struct Position {
        int x, y;

        bool operator==(Position const& other) const {
            return x == other.x && y == other.y;
        }
    };

    Position lastMousePosition{.x=0,.y=0};
    Position currentMousePosition{.x=0,.y=0};
};
} // namespace Nebulite::Graphics
#endif // GRAPHICS_RMLSYSTEMINTERFACE_HPP
