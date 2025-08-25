/**
 * @file GUI.h
 * 
 * Planned expansion for GUI-Element-Insertion into the Nebulite Renderer
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace 


namespace GlobalSpaceTreeExpansion {

/**
 * @class Nebulite::GlobalSpaceTreeExpansion::GUI
 * @brief GUI-Element-Insertion for the Nebulite Renderer.
 */
class GUI : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::GlobalSpace, GUI> {
public:
    using Wrapper<Nebulite::GlobalSpace, GUI>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
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
    Nebulite::ERROR_TYPE example(int argc, char* argv[]);
    
    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&GUI::example, "GUI-Example", "An example function to demonstrate GUI-Elements");
    }
};
}   // namespace GlobalSpaceTreeExpansion
}   // namespace Nebulite