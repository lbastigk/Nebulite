//------------------------------------------
// Includes

// Nebulite
#include "Module/Domain/Renderer/Input.hpp"
#include "Nebulite.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {

Constants::Event Input::updateHook() {
    moduleScope.set<bool>(Key::routineActivated, false);
    if (domain.isSdlInitialized()) {
        if (resetDeltaOnNextUpdate) {
            resetDeltaValues();
            resetDeltaOnNextUpdate = false;
        }
    }
    return Constants::Event::Success;
}

//------------------------------------------
// Private Functions

void Input::mapKeyNames() {
    // Mouse
    mouseCurrent = MouseValues{
        .x = moduleScope.getStableDoublePointer(Key::mouseCurrent + "X"),
        .y = moduleScope.getStableDoublePointer(Key::mouseCurrent + "Y"),
        .xScaled = moduleScope.getStableDoublePointer(Key::mouseCurrent + "scaledX"),
        .yScaled = moduleScope.getStableDoublePointer(Key::mouseCurrent + "scaledY"),
        .left = moduleScope.getStableDoublePointer(Key::mouseCurrent + "left"),
        .right = moduleScope.getStableDoublePointer(Key::mouseCurrent + "right")
    };
    mouseDelta = MouseValues{
        .x = moduleScope.getStableDoublePointer(Key::mouseDelta + "X"),
        .y = moduleScope.getStableDoublePointer(Key::mouseDelta + "Y"),
        .xScaled = moduleScope.getStableDoublePointer(Key::mouseDelta + "scaledX"),
        .yScaled = moduleScope.getStableDoublePointer(Key::mouseDelta + "scaledY"),
        .left = moduleScope.getStableDoublePointer(Key::mouseDelta + "left"),
        .right = moduleScope.getStableDoublePointer(Key::mouseDelta + "right")
    };

    // Keyboard
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
                auto currentPath = Key::keyboardCurrent + keyNames[scancode];
                auto deltaPath = Key::keyboardDelta + keyNames[scancode];
                deltaKey[scancode] = moduleScope.getStableDoublePointer(deltaPath);
                currentKey[scancode] = moduleScope.getStableDoublePointer(currentPath);
            }
        }
    }
}

void Input::addRoutines(){
    addRoutine(
        Utility::Coordination::TimedRoutine(
            [this]() -> void {
                // Only update if SDL is initialized
                if (domain.isSdlInitialized()) {
                    SDL_PumpEvents();
                    writeCurrentAndDeltaInputs();
                    moduleScope.set<bool>(Key::routineActivated, true);
                    resetDeltaOnNextUpdate = true; // Mark to reset deltas on next update
                }
            },
            10 /* ms */,
            Utility::Coordination::TimedRoutine::ConstructionMode::START_IMMEDIATELY
        ),
        RoutineUpdateMode::AFTER_UPDATE_HOOK
    );
}

namespace {

// Helper to calculate mouse state from SDL values
int calcMouseState(uint32_t const& key, uint32_t const& state) {
    return !!(key & state);
}

// Helper to calculate mouse delta from SDL values
int calcMouseDelta(uint32_t const& key, uint32_t const& currentState, uint32_t const& lastState) {
    return !!(key & currentState) - !!(key & lastState);
}

} // namespace

void Input::writeCurrentAndDeltaInputs() {
    //------------------------------------------
    // Mouse
    mouse.lastPosX = mouse.posX;
    mouse.lastPosY = mouse.posY;
    mouse.lastState = mouse.state;
    mouse.state = SDL_GetMouseState(&mouse.posX, &mouse.posY);

    // Cursor Position and state
    *mouseCurrent.x = static_cast<double>(mouse.posX);
    *mouseCurrent.y = static_cast<double>(mouse.posY);
    *mouseDelta.x = static_cast<double>(mouse.posX - mouse.lastPosX);
    *mouseDelta.y = static_cast<double>(mouse.posY - mouse.lastPosY);
    *mouseCurrent.left = calcMouseState(SDL_BUTTON_MASK(SDL_BUTTON_LEFT), mouse.state);
    *mouseCurrent.right = calcMouseState(SDL_BUTTON_MASK(SDL_BUTTON_RIGHT), mouse.state);
    *mouseDelta.left = calcMouseDelta(SDL_BUTTON_MASK(SDL_BUTTON_LEFT), mouse.state, mouse.lastState);
    *mouseDelta.right = calcMouseDelta(SDL_BUTTON_MASK(SDL_BUTTON_RIGHT), mouse.state, mouse.lastState);

    // Scaled positions
    *mouseCurrent.xScaled = static_cast<double>(mouse.posX)/static_cast<double>(domain.getWindowScale());
    *mouseCurrent.yScaled = static_cast<double>(mouse.posY)/static_cast<double>(domain.getWindowScale());
    *mouseDelta.xScaled = static_cast<double>(mouse.posX - mouse.lastPosX)/static_cast<double>(domain.getWindowScale());
    *mouseDelta.yScaled = static_cast<double>(mouse.posY - mouse.lastPosY)/static_cast<double>(domain.getWindowScale());

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
    *mouseDelta.x = 0.0;
    *mouseDelta.y = 0.0;
    *mouseDelta.left = 0.0;
    *mouseDelta.right = 0.0;

    // 2.) Keyboard
    for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++scancode) {
        if (!keyNames[scancode].empty()) {
            *deltaKey[scancode] = 0.0;
        }
    }
}

} // namespace Nebulite::Module::Domain::GlobalSpace
