/**
 * @file Input.hpp
 * @brief Input handling for the NEBULITE engine.
 */

#ifndef NEBULITE_DOMAINMODULE_RENDERER_INPUT_HPP
#define NEBULITE_DOMAINMODULE_RENDERER_INPUT_HPP

//------------------------------------------
// Includes

// External
#include <SDL3/SDL.h>    // Needed to access SDL input events

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::DomainModule::Renderer {
/**
 * @class Nebulite::DomainModule::Renderer::Input
 * @brief DomainModule for handling input events and states.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::Renderer, Input) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // None, input just updates the global doc based on SDL events

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::Renderer, Input) {
        mapKeyNames();
        addRoutines();
    }

    struct Key : Data::KeyGroup<"renderer.input."> {
        static auto constexpr keyboard = makeScoped("keyboard.");
        static auto constexpr keyboardCurrent = makeScoped("keyboard.current.");
        static auto constexpr keyboardDelta = makeScoped("keyboard.delta.");
        static auto constexpr mouse = makeScoped("mouse.");
        static auto constexpr mouseCurrent = makeScoped("mouse.current.");
        static auto constexpr mouseDelta = makeScoped("mouse.delta.");
        static auto constexpr routineActivated = makeScoped("polled");
    };

private:
    //---------------------------------
    // Functions

    /**
     * @brief Maps SDL scancodes to human-readable key names and sets double pointers.
     * @details This function populates the keyNames map with SDL scancode values
     *          as keys and their corresponding human-readable names as values.
     */
    void mapKeyNames();

    /**
     * @brief Adds all routines necessary for input handling
     */
    void addRoutines();

    /**
     * @brief Writes the current and delta input states to the global JSON structure.
     */
    void writeCurrentAndDeltaInputs();

    /**
     * @brief Resets all delta input values to zero.
     */
    void resetDeltaValues() const;

    //---------------------------------
    // Variables

    /**
     * @brief Flag to reset delta values on the next update.
     */
    bool resetDeltaOnNextUpdate = false;

    /**
     * @struct Mouse
     * 
     * @brief Stores current and last Position/State of the mouse.
     */
    struct Mouse {
        float posX = 0;
        float posY = 0;
        float lastPosX = 0;
        float lastPosY = 0;
        Uint32 lastState = 0;
        Uint32 state = 0;
    } mouse;

    /**
     * @brief Array to store key names corresponding to SDL scancodes.
     */
    std::string keyNames[SDL_SCANCODE_COUNT] = {""};

    /**
     * @brief Array to store previous key states.
     * 
     * We do not need to store current key states,
     * as SDL does that for us.
     */
    bool prevKey[SDL_SCANCODE_COUNT] = {false}; // Previous key states

    /**
     * @brief Array to store pointers to double values representing the delta states of keys.
     * Represents their values inside the global document.
     */
    double* deltaKey[SDL_SCANCODE_COUNT] = {nullptr}; // Pointers to delta key states in global doc

    /**
     * @brief Array to store pointers to double values representing the current states of keys.
     * Represents their values inside the global document.
     */
    double* currentKey[SDL_SCANCODE_COUNT] = {nullptr}; // Pointers to current key states in global doc
};
} // namespace Nebulite::DomainModule::Renderer
#endif // NEBULITE_DOMAINMODULE_RENDERER_INPUT_HPP
