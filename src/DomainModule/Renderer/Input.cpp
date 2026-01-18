#include "Nebulite.hpp"
#include "DomainModule/Renderer/Input.hpp"
#include "Utility/TimedRoutine.hpp"

namespace Nebulite::DomainModule::Renderer {

Constants::Error Input::update() {
    static Utility::TimedRoutine routine(
        [this]() -> void {
            SDL_PumpEvents();
            writeCurrentAndDeltaInputs();
            resetDeltaOnNextUpdate = true; // Mark to reset deltas on next update
        },
        10 /* ms */,
        Utility::TimedRoutine::ConstructionMode::START_IMMEDIATELY
    );

    //------------------------------------------
    // Only update if SDL is initialized
    if (domain.isSdlInitialized()) {
        if (resetDeltaOnNextUpdate) {
            resetDeltaValues();
            resetDeltaOnNextUpdate = false;
        }
        routine.update();
    }
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Private Functions

void Input::map_key_names() {
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++
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
                auto currentPath = moduleScope.getRootScope() + "keyboard.current." + keyNames[scancode];
                auto deltaPath = moduleScope.getRootScope() + "keyboard.delta." + keyNames[scancode];
                deltaKey[scancode] = moduleScope.getStableDoublePointer(deltaPath);
                currentKey[scancode] = moduleScope.getStableDoublePointer(currentPath);
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
    moduleScope.set(moduleScope.getRootScope() + "mouse.current.X", static_cast<double>(mouse.posX));
    moduleScope.set(moduleScope.getRootScope() + "mouse.current.Y", static_cast<double>(mouse.posY));
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.X", static_cast<double>(mouse.posX - mouse.lastPosX));
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.Y", static_cast<double>(mouse.posY - mouse.lastPosY));
    moduleScope.set(moduleScope.getRootScope() + "mouse.current.left", calcMouseState(SDL_BUTTON_MASK(SDL_BUTTON_LEFT), mouse.state));
    moduleScope.set(moduleScope.getRootScope() + "mouse.current.right", calcMouseState(SDL_BUTTON_MASK(SDL_BUTTON_RIGHT), mouse.state));
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.left", calcMouseDelta(SDL_BUTTON_MASK(SDL_BUTTON_LEFT), mouse.state, mouse.lastState));
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.right", calcMouseDelta(SDL_BUTTON_MASK(SDL_BUTTON_RIGHT), mouse.state, mouse.lastState));

    // Scaled positions
    moduleScope.set(moduleScope.getRootScope() + "mouse.current.scaledX", static_cast<double>(mouse.posX)/static_cast<double>(domain.getWindowScale()));
    moduleScope.set(moduleScope.getRootScope() + "mouse.current.scaledY", static_cast<double>(mouse.posY)/static_cast<double>(domain.getWindowScale()));
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.scaledX", static_cast<double>(mouse.posX - mouse.lastPosX)/static_cast<double>(domain.getWindowScale()));
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.scaledY", static_cast<double>(mouse.posY - mouse.lastPosY)/static_cast<double>(domain.getWindowScale()));

    //------------------------------------------
    // Keyboard
    auto const* keyState = SDL_GetKeyboardState(nullptr);
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++scancode) {
        if (!keyNames[scancode].empty()) {
            // Retrieve state, store previous state
            bool const currentPressed = keyState[scancode];
            bool const prevPressed = prevKey[scancode];
            prevKey[scancode] = currentPressed;

            int delta = 0;
            if (currentPressed && !prevPressed)
                delta = 1;
            else if (!currentPressed && prevPressed)
                delta = -1;

            *currentKey[scancode] = static_cast<double>(currentPressed);
            *deltaKey[scancode] = static_cast<double>(delta);
        }
    }
}

void Input::resetDeltaValues() const {
    // 1.) Mouse
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.X", 0);
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.Y", 0);
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.left", 0);
    moduleScope.set(moduleScope.getRootScope() + "mouse.delta.right", 0);

    // 2.) Keyboard
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++
         scancode) {
        if (!keyNames[scancode].empty()) {
            std::string deltaPath =
                "keyboard.delta." + keyNames[scancode];
            moduleScope.set<int>(moduleScope.getRootScope() + deltaPath, 0);
        }
    }
}

} // namespace Nebulite::DomainModule::GlobalSpace
