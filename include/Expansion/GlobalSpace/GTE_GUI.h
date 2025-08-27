/**
 * @file GUI.h
 * 
 * Planned expansion for GUI-Element-Insertion into the Nebulite Renderer
 */

#pragma once

#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/ExpansionWrapper.h"

//----------------------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class GlobalSpace; // Forward declaration of domain class GlobalSpace
    }
}

//----------------------------------------------------------
namespace Nebulite {
namespace Expansion {
namespace GlobalSpace {
/**
 * @class Nebulite::Expansion::GlobalSpace::GUI
 * @brief Expansion for creating GUI elements and queueing them in the renderer pipeline.
 */
class GUI : public Nebulite::Interaction::Execution::ExpansionWrapper<Nebulite::Core::GlobalSpace, GUI> {
public:
    using ExpansionWrapper<Nebulite::Core::GlobalSpace, GUI>::ExpansionWrapper; // Templated constructor from Wrapper, call this->setupBindings()

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
    Nebulite::Constants::ERROR_TYPE example(int argc, char* argv[]);
    
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
}   // namespace GlobalSpace
}   // namespace Expansion
}   // namespace Nebulite