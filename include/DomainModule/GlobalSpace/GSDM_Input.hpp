/**
 * @file GSDM_Input.hpp
 * @brief Input handling for the NEBULITE engine.
 *
 * This file contains the input handling logic for the NEBULITE engine,
 * including keyboard and mouse input processing.
 * 
 * @todo Move to Renderer Domain, since it relies on SDL events!
 */

#ifndef NEBULITE_DOMAINMODULE_GLOBALSPACE_GSDM_INPUT_HPP
#define NEBULITE_DOMAINMODULE_GLOBALSPACE_GSDM_INPUT_HPP

//------------------------------------------
// Includes

// General
#include <SDL.h>    // Needed to access SDL input events

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"
#include "Utility/TimeKeeper.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Core{
    class GlobalSpace; // Forward declaration of domain class GlobalSpace
}

//------------------------------------------
namespace Nebulite::DomainModule::GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::Input
 * @brief DomainModule for handling input events and states.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, Input){
public:
    /**
     * @brief Updates the input states of mouse and keyboard 
     * stored in the global document by polling SDL events.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    // None, input just updates the global doc based on SDL events

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, Input){
        // Mapping key names
        map_key_names();
    }

private:
    //---------------------------------
    // Functions

    /**
     * @brief Maps SDL scancodes to human-readable key names and sets double pointers.
     * 
     * This function populates the keyNames map with SDL scancode values
     * as keys and their corresponding human-readable names as values.
     */
    void map_key_names();

    /**
     * @brief Writes the current and delta input states to the global JSON structure.
     */
    void write_current_and_delta_inputs();

    /**
     * @brief Resets all delta input values to zero.
     */
    void reset_delta_values();

    //---------------------------------
    // Variables

    /**
     * @brief Timer for input handling.
     * 
     * If last update is over a threshold, we poll for input again.
     */
    std::shared_ptr<Nebulite::Utility::TimeKeeper> RendererPollTime;

    /**
     * @brief If the timer was initialized.
     */
    bool timerInitialized = false;

    /**
     * @brief Flag to reset delta values on the next update.
     */
    bool reset_delta_on_next_update = false;

    /**
     * @struct Mouse
     * 
     * @brief Stores current and last Position/State of the mouse.
     */
    struct Mouse {
        int posX = 0;
        int posY = 0;
        int lastPosX = 0;
        int lastPosY = 0;
        Uint32 lastState;
        Uint32 state;
    } mouse;

    /**
     * @brief Array to store key names corresponding to SDL scancodes.
     */
    std::string keyNames[SDL_NUM_SCANCODES] = {""};

    /**
     * @brief Array to store previous key states.
     * 
     * We do not need to store current key states,
     * as SDL does that for us.
     */
    bool prevKey[SDL_NUM_SCANCODES] = {false};      // Previous key states

    /**
     * @brief Array to store pointers to double values representing the delta states of keys.
     * Represents their values inside the global document.
     */
    double* deltaKey[SDL_NUM_SCANCODES] = {nullptr}; // Pointers to delta key states in global doc

    /**
     * @brief Array to store pointers to double values representing the current states of keys.
     * Represents their values inside the global document.
     */
    double* currentKey[SDL_NUM_SCANCODES] = {nullptr}; // Pointers to current key states in global doc
};
}   // namespace Nebulite::DomainModule::GlobalSpace
#endif // NEBULITE_DOMAINMODULE_GLOBALSPACE_GSDM_INPUT_HPP