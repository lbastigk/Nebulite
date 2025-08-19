/**
 * @file GUI.h
 * 
 * Planned extension for GUI-Element-Insertion into the Nebulite Renderer
 */

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class GlobalSpace; // Forward declaration of domain class GlobalSpace 
namespace GlobalSpaceTreeExpansion {
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
     */
    Nebulite::ERROR_TYPE example(int argc, char* argv[]);
    
    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        bindFunction(&GUI::example, "GUI-Example", "An example function to demonstrate GUI-Elements");
    }
};
}
}