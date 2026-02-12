//------------------------------------------
// Includes

// Standard library
#include <cstdint>

// Nebulite
#include "Nebulite.hpp"
#include "Constants/ErrorTypes.hpp"
#include "Core/Renderer.hpp"
#include "DomainModule/Renderer/Console.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {

void Console::keyTriggerSubmit() {
    if (std::string const command = textInput.submit(); !command.empty()) {
        // Parse command on global level for full access to all functions
        if (auto const err = Global::instance().parseStr(std::string(__FUNCTION__) + " " + command); err != Constants::ErrorTable::NONE()) {
            // Cannot escalate error further, print to cerr
            Error::println(err.getDescription());
        }
    }
    outputScrollingOffset = 0; // Reset scrolling to bottom on new input
}

void Console::keyTriggerScrollUp() {
    if (outputScrollingOffset < UINT16_MAX - 1) {
        outputScrollingOffset += 1;
    }
}

void Console::keyTriggerScrollDown() {
    if (outputScrollingOffset > 0) {
        outputScrollingOffset -= 1;
    }
}

void Console::keyTriggerZoomIn() const {
    // Make sure that ctrl is held
    if (!(SDL_GetModState() & SDL_KMOD_CTRL))
        return;
    if (auto const err = domain.parseStr(__FUNCTION__ + std::string(" ") + std::string(consoleZoom_name) + " in"); err != Constants::ErrorTable::NONE()) {
        Error::println("Error: Failed to zoom into console: ", err.getDescription());
    }
}

void Console::keyTriggerZoomOut() const {
    // Make sure that ctrl is held
    if (!(SDL_GetModState() & SDL_KMOD_CTRL))
        return;
    if (auto const err = domain.parseStr(__FUNCTION__ + std::string(" ") + std::string(consoleZoom_name) + " out"); err != Constants::ErrorTable::NONE()) {
        Error::println("Error: Failed to zoom out console: ", err.getDescription());
    }
}

void Console::processKeyDownEvent(SDL_KeyboardEvent const& key) {
switch (key.key) {
        //------------------------------------------
        // Text input manipulation

        // Remove last character on backspace
        case SDLK_BACKSPACE:
            textInput.backspace();
            break;

        // Submit command on Enter
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            keyTriggerSubmit();
            break;

        // Cursor movement
        case SDLK_LEFT:
            textInput.moveCursorLeft();
            break;
        case SDLK_RIGHT:
            textInput.moveCursorRight();
            break;

        /**
         * @todo: Implement copy/paste functionality with CTRL + C/V
         * perhaps integrate a clipboard manager into Renderer class?
         * Then, it could be used for both the console and other TextInput instances.
         *
         * For this to work properly, we should consider text handling to be only registered by one instance at a time.
         * Perhaps we can move every keytrigger event to Renderer, and have it forward events to the active TextInput instance.
         *
         * Then we may have functions to move focus to objects:
         *
         * for Domain RenderObject:
         * textfocus self
         *
         * which itself sends a "textfocus id" to the "bus" that triggers on the scope of the Renderer by using:
         *
         * textfocus id <renderer_object_id>
         *
         * Then, we could have more textfocus additions like:
         * textfocus gui <gui_element_id>
         * textfocus console force on/off
         * etc.
         *
         * Then, we could have Renderer::setActiveTextInput(shared<TextInput>) which overwrites the current active text input, if its not forced.
         * And on Renderer::update(), we forward all text input and key events to that active TextInput instance.
         * Meaning this entire function would be moved to TextInput class, and Console would just set itself as active when in console mode.
         *
         * If a textfocus is active, we may wish to disable normal key processing from Input DomainModule.
         *
         * For this to work, we need to modify TextInput into two modes:
         * - On submit, store text in vector (for console)
         * - On submit, lose focus           (for single-line text inputs)
         *
         * This requires a big rework of TextInput class, we may wish to extract the core functionality of single-line text input with cursor to a separate class.
         * - TextInput for core functionality
         * - Use Capture class directly for console readout
         * Since the current TextInput class has a lot logic that is the same as Capture's output log.
         * Only thing left is to add a Capture type for commands entered.
         */

        //------------------------------------------
        // UP/DOWN to cycle through past commands
        case SDLK_UP:
            textInput.history_up();
            break;
        case SDLK_DOWN:
            textInput.history_down();
            break;

        //------------------------------------------
        // Scroll through output with PAGE UP/DOWN
        case SDLK_PAGEUP:
            keyTriggerScrollUp();
            break;
        case SDLK_PAGEDOWN:
            keyTriggerScrollDown();
            break;

        //------------------------------------------
        // Zoom in/out with +/- keys
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
            keyTriggerZoomIn();
            break;

        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            keyTriggerZoomOut();
            break;

        //------------------------------------------
        default:
            break;

    }
}

void Console::processEvents() {
    for (auto const& event : *events) {
        switch (event.type) {
        case SDL_EVENT_TEXT_INPUT:
            // Do not append if ctrl is held (to allow copy/paste and other shortcuts)
            if (SDL_GetModState() & SDL_KMOD_CTRL) {
                break;
            }
            textInput.append(event.text.text);
            break;
        case SDL_EVENT_KEY_DOWN:
            processKeyDownEvent(event.key);
            break;
        default:
            break;
        }
    }
}

void Console::processMode() {
    if (consoleMode) {
        // Render texture and attach
        renderConsole();

        // Check if texture is valid
        if (!consoleTexture.texture_ptr) {
            Error::println("Could not attach Console: Console texture is null!");
            return;
        }

        // Attach texture above UI layer
        (void)domain.attachTextureAboveLayer(
            Core::Environment::Layer::UI,
            "console_overlay",
            consoleTexture.texture_ptr,
            &consoleTexture.rect
            );

        // Skip updating the renderer for this frame, as we are in console mode
        domain.skipUpdateNextFrame();
    } else {
        // Clear texture and detach
        (void)domain.detachTextureAboveLayer(
            Core::Environment::Layer::UI,
            "console_overlay"
            );
        if (consoleTexture.texture_ptr) {
            SDL_DestroyTexture(consoleTexture.texture_ptr);
            consoleTexture.texture_ptr = nullptr;
        }
    }
}


} // namespace Nebulite::DomainModule::Renderer
