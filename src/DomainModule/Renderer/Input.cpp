#include "DomainModule/Renderer/Input.hpp"
#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Renderer {

Constants::Error Input::update() {
    //------------------------------------------
    // Only update if SDL is initialized
    if (domain.isSdlInitialized()) {
        if (!timerInitialized) {
            // Starting Polling timer
            RendererPollTime = std::make_shared<Utility::TimeKeeper>();
            RendererPollTime->update(); // Initial update to set t and dt
            RendererPollTime->start();
            RendererPollTime->update(); // Initial update to set t and dt
            timerInitialized = true;
        }

        //------------------------------------------
        // 2-Step Update of Input state

        // 1.) Setting all delta values to 0, so they're only on delta for one frame
        if (resetDeltaOnNextUpdate) {
            resetDeltaValues();
            resetDeltaOnNextUpdate = false;
        }

        // 2.) Polling mouse and keyboard state
        // Update every 10 ms
        // Too much polling time for current benchmarks, if we update each frame
        // later on with fixed framerates of < 250 FPS perhaps not that big of a deal
        if (RendererPollTime->projected_dt() > 10) {
            // Updating inputs
            RendererPollTime->update();
            SDL_PumpEvents();
            writeCurrentAndDeltaInputs();

            // Reset delta values on next update
            // Since we only write input current and delta every 10ms,
            // we don't want to reset delta values every frame
            // This would take too much computational time for no reason
            resetDeltaOnNextUpdate = true;
        }
    }
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Private Functions

void Input::map_key_names() {
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++
         scancode) {
        char const* nameRaw = SDL_GetScancodeName(
            static_cast<SDL_Scancode>(scancode));

        if (nameRaw && nameRaw[0] != '\0') {
            std::string keyName = nameRaw;

            // Normalize key name: lowercase, spaces to underscores
            std::ranges::for_each(keyName, [](char& ch) {
                auto const uch = static_cast<unsigned char>(ch);
                ch = static_cast<char>(std::tolower(uch));
                if (ch == ' ')
                    ch = '_';
            });

            // Don't add if there are special chars in Nebulite::Constants::keyName
            if (!Utility::StringHandler::containsAnyOf(
                keyName, Data::JSON::reservedCharacters)) {
                keyNames[scancode] = keyName;

                // Paths
                auto currentPath = getDoc().getRootScope() + "keyboard.current." + keyNames[scancode];
                auto deltaPath = getDoc().getRootScope() + "keyboard.delta." + keyNames[scancode];
                deltaKey[scancode] = getDoc().getStableDoublePointer(deltaPath);
                currentKey[scancode] = getDoc().getStableDoublePointer(currentPath);
            }
        }
    }
}

namespace {
int calcMouseState(uint32_t const& key, uint32_t const& state) {
    return !!(key & state);
}

int calcMouseDelta(uint32_t const& key, uint32_t const& currentState, uint32_t const& lastState) {
    return !!(key & currentState) - !!(key & lastState);
}
}

void Input::writeCurrentAndDeltaInputs() {
    //------------------------------------------
    // Mouse
    mouse.lastPosX = mouse.posX;
    mouse.lastPosY = mouse.posY;
    mouse.lastState = mouse.state;
    mouse.state = SDL_GetMouseState(&mouse.posX, &mouse.posY);

    // Cursor Position and state
    getDoc().set(getDoc().getRootScope() + "mouse.current.X", mouse.posX);
    getDoc().set(getDoc().getRootScope() + "mouse.current.Y", mouse.posY);
    getDoc().set(getDoc().getRootScope() + "mouse.delta.X", mouse.posX - mouse.lastPosX);
    getDoc().set(getDoc().getRootScope() + "mouse.delta.Y", mouse.posY - mouse.lastPosY);
    getDoc().set(getDoc().getRootScope() + "mouse.current.left", calcMouseState(SDL_BUTTON(SDL_BUTTON_LEFT), mouse.state));
    getDoc().set(getDoc().getRootScope() + "mouse.current.right", calcMouseState(SDL_BUTTON(SDL_BUTTON_RIGHT), mouse.state));
    getDoc().set(getDoc().getRootScope() + "mouse.delta.left", calcMouseDelta(SDL_BUTTON(SDL_BUTTON_LEFT), mouse.state, mouse.lastState));
    getDoc().set(getDoc().getRootScope() + "mouse.delta.right", calcMouseDelta(SDL_BUTTON(SDL_BUTTON_RIGHT), mouse.state, mouse.lastState));

    //------------------------------------------
    // Keyboard
    uint8_t const* keyState = SDL_GetKeyboardState(nullptr);
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++
         scancode) {
        if (!keyNames[scancode].empty()) {
            // Retrieve state, store previous state
            // If key is currently pressed
            bool const currentPressed = keyState[scancode] != 0;
            bool const prevPressed = prevKey[scancode];
            prevKey[scancode] = currentPressed;

            // Compute delta:
            // ->  1 = pressed now but not before
            // -> -1 = released now but was pressed before,
            // ->  0 = no change
            int delta = 0;
            if (currentPressed && !prevPressed)
                delta = 1;
            else if (!currentPressed && prevPressed)
                delta = -1;

            // Set current state (true/false as int)
            *currentKey[scancode] = static_cast<double>(currentPressed);

            // Set delta
            *deltaKey[scancode] = static_cast<double>(delta);
        }
    }
}

void Input::resetDeltaValues() const {
    // 1.) Mouse
    getDoc().set(getDoc().getRootScope() + "mouse.delta.X", 0);
    getDoc().set(getDoc().getRootScope() + "mouse.delta.Y", 0);
    getDoc().set(getDoc().getRootScope() + "mouse.delta.left", 0);
    getDoc().set(getDoc().getRootScope() + "mouse.delta.right", 0);

    // 2.) Keyboard
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_NUM_SCANCODES; ++
         scancode) {
        if (!keyNames[scancode].empty()) {
            std::string deltaPath =
                "keyboard.delta." + keyNames[scancode];
            getDoc().set<int>(getDoc().getRootScope() + deltaPath, 0);
        }
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace
