//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint> // NOLINT
#include <string>

// External
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

// Nebulite
#include "Constants/Event.hpp"
#include "Core/GlobalSpace.hpp"
#include "Data/Document/JSON.hpp"
#include "Module/Domain/Renderer/Input.hpp"
#include "Utility/Coordination/TimedRoutine.hpp"
#include "Utility/StringHandler.hpp"

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
        .x = moduleScope.getStableDoublePointer(Key::mouseCurrent.addMember("X")),
        .y = moduleScope.getStableDoublePointer(Key::mouseCurrent.addMember("Y")),
        .xScaled = moduleScope.getStableDoublePointer(Key::mouseCurrent.addMember("scaledX")),
        .yScaled = moduleScope.getStableDoublePointer(Key::mouseCurrent.addMember("scaledY")),
        .left = moduleScope.getStableDoublePointer(Key::mouseCurrent.addMember("left")),
        .right = moduleScope.getStableDoublePointer(Key::mouseCurrent.addMember("right"))
    };
    mouseDelta = MouseValues{
        .x = moduleScope.getStableDoublePointer(Key::mouseDelta.addMember("X")),
        .y = moduleScope.getStableDoublePointer(Key::mouseDelta.addMember("Y")),
        .xScaled = moduleScope.getStableDoublePointer(Key::mouseDelta.addMember("scaledX")),
        .yScaled = moduleScope.getStableDoublePointer(Key::mouseDelta.addMember("scaledY")),
        .left = moduleScope.getStableDoublePointer(Key::mouseDelta.addMember("left")),
        .right = moduleScope.getStableDoublePointer(Key::mouseDelta.addMember("right"))
    };

    // Keyboard
    for (std::size_t scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++
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
                keyboardState.keyNames[scancode] = keyName;

                // Paths
                auto currentPath = Key::keyboardCurrent.addMember(keyboardState.keyNames[scancode]);
                auto deltaPath = Key::keyboardDelta.addMember(keyboardState.keyNames[scancode]);
                keyboardState.deltaKey[scancode] = moduleScope.getStableDoublePointer(deltaPath);
                keyboardState.currentKey[scancode] = moduleScope.getStableDoublePointer(currentPath);
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
int calcMouseState(std::uint32_t const key, std::uint32_t const state) {
    return !!(key & state);
}

// Helper to calculate mouse delta from SDL values
int calcMouseDelta(std::uint32_t const key, std::uint32_t const currentState, std::uint32_t const lastState) {
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
    for (std::size_t scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++scancode) {
        if (!keyboardState.keyNames[scancode].empty()) {
            // Retrieve state, store previous state
            bool const currentPressed = keyState[scancode];
            bool const prevPressed = keyboardState.prevKey[scancode];
            keyboardState.prevKey[scancode] = currentPressed;

            int delta = 0;
            if (currentPressed && !prevPressed)
                delta = 1;
            else if (!currentPressed && prevPressed)
                delta = -1;

            *keyboardState.currentKey[scancode] = static_cast<double>(currentPressed);
            *keyboardState.deltaKey[scancode] = static_cast<double>(delta);
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
    for (std::size_t scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT; ++scancode) {
        if (!keyboardState.keyNames[scancode].empty()) {
            *keyboardState.deltaKey[scancode] = 0.0;
        }
    }
}

} // namespace Nebulite::Module::Domain::Renderer
