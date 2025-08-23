#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration of Domain class RenderObject

namespace RenderObjectTreeExpansion {
class Layout : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, Layout> {
public:
    using Wrapper<Nebulite::RenderObject, Layout>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE alignText(int argc, char* argv[]);
    Nebulite::ERROR_TYPE makeBox(int argc, char* argv[]);

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
}   // namespace RenderObjectTreeExpansion
}   // namespace Nebulite