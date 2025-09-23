/**
 * @file GDM_Input.hpp
 * @brief Input handling for the NEBULITE engine.
 *
 * This file contains the input handling logic for the NEBULITE engine,
 * including keyboard and mouse input processing.
 */

#pragma once

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
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace GlobalSpace {
/**
 * @class Nebulite::DomainModule::GlobalSpace::GUI
 * @brief DomainModule for creating GUI elements and queueing them in the renderer pipeline.
 */
class Input : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
public:
    /**
     * @brief Updates the input states of mouse and keyboard 
     * stored in the global document by polling SDL events.
     */
    void update();

    //------------------------------------------
    // Available Functions

    // None, input just updates the global doc based on SDL events

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Input(std::string moduleName, Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
        // Starting Polling timer
        RendererPollTime = std::make_shared<Nebulite::Utility::TimeKeeper>();
        RendererPollTime->update(); // Initial update to set t and dt
        RendererPollTime->start();
        RendererPollTime->update(); // Initial update to set t and dt

        // Mapping key names
        map_key_names();
    }

private:
    //---------------------------------
    // Functions

    /**
     * @brief Maps SDL scancodes to human-readable key names.
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
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite