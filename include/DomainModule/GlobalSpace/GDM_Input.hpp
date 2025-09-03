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
#include "Interaction/Execution/DomainModuleWrapper.hpp"
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
class Input : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Core::GlobalSpace, Input> {
public:
    using DomainModuleWrapper<Nebulite::Core::GlobalSpace, Input>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //------------------------------------------
    // Available Functions

    // TODO: Forced global values here
    /**
     * @brief Forces a global variable to a specific value
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     * 
     * @todo move to GDM_GlobalForce, as this is not just for input
     * Alternatively, this might make sense as a JSON Module!
     */
    Nebulite::Constants::ERROR_TYPE forceGlobal(int argc, char* argv[]);

    /**
     * @brief Clears all forced global variables
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     * 
     * @todo move to GDM_GlobalForce, as this is not just for input
     * Alternatively, this might make sense as a JSON Module!
     */
    Nebulite::Constants::ERROR_TYPE clearForceGlobal(int argc, char* argv[]);

    //------------------------------------------
    // Setup
    void setupBindings() {
        // Starting Polling timer
        RendererPollTime.start();

        // Binding
        bindFunction(&Input::forceGlobal,         "force-global",         "Force a global variable to a value: force-global <key> <value>");
        bindFunction(&Input::clearForceGlobal,    "force-global-clear",   "Clear all forced global variables");
    }

private:
    void pollEvent();

    //---------------------------------
    // Private vars

    Nebulite::Utility::TimeKeeper RendererPollTime;	// Used for Polling timing
    bool reset_delta = false; 		                // Reset delta values on next update

    struct Mouse {
        int posX = 0;
        int posY = 0;
        int lastPosX = 0;
        int lastPosY = 0;
        Uint32 lastState;
        Uint32 state;
    } mouse;

	//std::vector<Uint8> prevKeyState;
    absl::flat_hash_map<int, uint8_t> prevKeyState; // scancode -> keystate

    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite