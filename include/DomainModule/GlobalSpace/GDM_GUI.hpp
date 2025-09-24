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
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Example function for GUI elements.
     * 
     * Idea is to create a simple GUI-Element with Dear ImGui.
     * 
     * @param argc The argument count
     * @param argv The argument vector: no arguments available
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error example(int argc, char* argv[]);
    
    //------------------------------------------
    // Setup

    /**
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::GlobalSpace, GUI){
        bindSubtree("gui", "Functions to create GUI elements");
        bindFunction(&GUI::example, "gui example", "An example function to demonstrate GUI-Elements");
    }

private:
    SDL_Renderer* gui_renderer; // GUI should have its own renderer?
};
}   // namespace GlobalSpace
}   // namespace DomainModule
}   // namespace Nebulite