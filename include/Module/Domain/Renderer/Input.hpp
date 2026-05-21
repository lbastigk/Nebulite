/**
 * @file Input.hpp
 * @brief Input handling for the NEBULITE engine.
 */

#ifndef MODULE_DOMAIN_RENDERER_INPUT_HPP
#define MODULE_DOMAIN_RENDERER_INPUT_HPP

//------------------------------------------
// Includes

// External
#include <SDL3/SDL.h>    // Needed to access SDL input events

// Nebulite
#include "Data/Document/KeyGroup.hpp"
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class Renderer;
} // namespace Nebulite::Core

//------------------------------------------
namespace Nebulite::Module::Domain::Renderer {
/**
 * @class Nebulite::Module::Domain::Renderer::Input
 * @brief DomainModule for handling input events and states.
 */
class Input final : public Base::DomainModule<Core::Renderer> {
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
    explicit Input(ConstructorParams const& params) : DomainModule(params) {
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
     * @struct KeyboardState
     * @brief Stores key names, current states, previous states, and delta states for keyboard input.
     */
    struct KeyboardState {
        /**
         * @brief Array to store key names corresponding to SDL scancodes.
         */
        std::array<std::string, SDL_SCANCODE_COUNT> keyNames;

        /**
         * @brief Array to store pointers to double values representing the current states of keys.
         * @details Represents their values inside the global document.
         */
        std::array<double*, SDL_SCANCODE_COUNT> currentKey = {nullptr};

        /**
         * @brief Array to store previous key states.
         * @details We do not need to store current key states, as SDL does that for us.
         */
        std::array<bool, SDL_SCANCODE_COUNT> prevKey = {false};

        /**
         * @brief Array to store pointers to double values representing the delta states of keys.
         * @details Represents their values inside the global document.
         */
        std::array<double*, SDL_SCANCODE_COUNT> deltaKey = {nullptr};
    } keyboardState;

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
     * @brief Mouse values
     */
    struct MouseValues {
        double* x;
        double* y;
        double* xScaled;
        double* yScaled;
        double* left;
        double* right;
    };

    MouseValues mouseCurrent{};
    MouseValues mouseDelta{};
};
} // namespace Nebulite::Module::Domain::Renderer
#endif // MODULE_DOMAIN_RENDERER_INPUT_HPP
