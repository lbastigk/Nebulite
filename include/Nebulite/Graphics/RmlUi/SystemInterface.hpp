#ifndef NEBULITE_GRAPHICS_RMLUI_SYSTEMINTERFACE_HPP
#define NEBULITE_GRAPHICS_RMLUI_SYSTEMINTERFACE_HPP

//------------------------------------------
// Includes

// External
#include <RmlUi/Config/Config.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi_Platform_SDL.h>
#include <SDL3/SDL_video.h>

// Nebulite
#include "Nebulite/Math/Vec2.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Graphics {
/**
 * @brief Custom interface to integrate RmlUis SDL SystemInterface with Nebulites capture system
 */
class RmlSystemInterface final : public SystemInterface_SDL {
public:
    RmlSystemInterface(SDL_Window* w, Utility::Io::Capture& c);

    void update(int mousePositionX, int mousePositionY);

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
     * @param cursorName The name of the cursor
     */
    void SetMouseCursor(Rml::String const& cursorName) override ;

private:
    Utility::Io::Capture& capture;
    bool logEnabled = true;

    using Position = Math::Vec2<int>;

    Position lastMousePosition{
        .x=0,
        .y=0,
    };
    Position currentMousePosition{
        .x=0,
        .y=0,
    };
};
} // namespace Nebulite::Graphics
#endif // NEBULITE_GRAPHICS_RMLUI_SYSTEMINTERFACE_HPP
