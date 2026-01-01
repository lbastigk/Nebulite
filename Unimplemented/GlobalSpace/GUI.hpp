/**
 * @file GUI.hpp
 * 
 * Planned DomainModule for GUI-Element-Insertion into the Nebulite Renderer
 */

#pragma once

//------------------------------------------
// Includes

// General
#include <SDL.h>

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

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
NEBULITE_DOMAINMODULE(Nebulite::Core::GlobalSpace, GUI) {
public:
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Example function for GUI elements.
     * 
     * Idea is to create a simple GUI-Element with Dear ImGui.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occurred on command execution
     */
    Nebulite::Constants::Error example(int argc, char* argv[]);
    
    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, GUI){
        bindCategory("gui", "Functions to create GUI elements");
        BINDFUNCTION(&GUI::example, "gui example", "An example function to demonstrate GUI-Elements\n");
    }

private:
    SDL_Renderer* gui_renderer; // GUI should have its own renderer?
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite