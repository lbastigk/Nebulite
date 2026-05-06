#ifndef NEBULITE_GRAPHICS_RML_SYSTEM_INTERFACE_HPP
#define NEBULITE_GRAPHICS_RML_SYSTEM_INTERFACE_HPP

//------------------------------------------
// Includes

// External
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

    void update();

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
        explicit Cursor(SDL_SystemCursor const& id) : data(SDL_CreateSystemCursor(id)) {
            usageTracker.start();
            usageTracker.update();
        }

        [[nodiscard]] SDL_Cursor* get() const {
            return data;
        }

        void update() {
            usageTracker.update();
        }

        uint64_t dt() {
            return usageTracker.projected_dt();
        }
    };

    struct AvailableCursors {
        Cursor general{SDL_SYSTEM_CURSOR_DEFAULT};
        Cursor move{SDL_SYSTEM_CURSOR_MOVE};
        Cursor pointer{SDL_SYSTEM_CURSOR_POINTER};
        Cursor resize{SDL_SYSTEM_CURSOR_NWSE_RESIZE};
        Cursor cross{SDL_SYSTEM_CURSOR_CROSSHAIR};
        Cursor text{SDL_SYSTEM_CURSOR_TEXT};
        Cursor unavailable{SDL_SYSTEM_CURSOR_NOT_ALLOWED};
    } availableCursors;
};
} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_RML_SYSTEM_INTERFACE_HPP
