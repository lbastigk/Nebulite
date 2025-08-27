/**
 * @file RTE_Layout.h
 * 
 * Layout management for the RenderObject tree expansion.
 */

#pragma once

#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/ExpansionWrapper.h"

//----------------------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Core{
        class RenderObject; // Forward declaration of domain class RenderObject
    }
}

//----------------------------------------------------------
namespace Nebulite{
namespace Expansion{
namespace RenderObject{
/**
 * @class Nebulite::Expansion::RenderObject::Layout
 * @brief Layout management for the RenderObject tree expansion.
 */
class Layout : public Nebulite::Interaction::Execution::ExpansionWrapper<Nebulite::Core::RenderObject, Layout> {
public:
    using ExpansionWrapper<Nebulite::Core::RenderObject, Layout>::ExpansionWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    /**
     * @brief Aligns text to object dimensions
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE alignText(int argc, char* argv[]);

    /**
     * @brief Creates a box based on text dimensions
     * 
     * @param argc The argument count
     * @param argv The argument vector: ...
     * @return Potential errors that occured on command execution
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE makeBox(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        bindFunction(&Layout::alignText, "align-text", "Aligns text to object dimensions");
        bindFunction(&Layout::makeBox, "make-box", "Creates a box based on text dimensions");
    }
};
}   // namespace Expansion
}   // namespace RenderObject
}   // namespace Nebulite