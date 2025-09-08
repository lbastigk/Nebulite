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
class InputNebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace> {
public:
    void update();

    //------------------------------------------
    // Available Functions

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    Input(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>* funcTreePtr) 
    : DomainModule(domain, funcTreePtr) {
        // Starting Polling timer
        RendererPollTime = std::make_shared<Nebulite::Utility::TimeKeeper>();
        RendererPollTime->update(); // Initial update to set t and dt
        RendererPollTime->start();
        RendererPollTime->update(); // Initial update to set t and dt

        // Mapping key names
        map_key_names();
    }

private:
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
    // Private vars

    // Used for determining when to poll inputs
    std::shared_ptr<Nebulite::Utility::TimeKeeper> RendererPollTime; // Timer for input handling
    //Nebulite::Utility::TimeKeeper RendererPollTime;	// Used for determining when to poll inputs
    
    bool reset_delta_on_next_update = false; 		// Making sure delta values are only active for one frame

    // Mouse state
    struct Mouse {
        int posX = 0;
        int posY = 0;
        int lastPosX = 0;
        int lastPosY = 0;
        Uint32 lastState;
        Uint32 state;
    } mouse;

    // Map of SDL Scancode to key name
    std::string keyNames[SDL_NUM_SCANCODES] = {""};
    
    // Keyboard state
    // We do not need to store current key states,
    // as SDL does that for us.
    bool prevKey[SDL_NUM_SCANCODES] = {false};      // Previous key states
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite