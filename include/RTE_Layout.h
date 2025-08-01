#pragma once

#include "ErrorTypes.h"
#include "RT_ExpansionWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace RenderObjectTreeExpansion {
class Layout : public Wrapper<Layout> {
public:
    using Wrapper<Layout>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE alignText(int argc, char* argv[]);
    Nebulite::ERROR_TYPE makeBox(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions

    void setupBindings(){
        bindFunction(&Layout::alignText, "align-text", "Aligns text to object dimensions");
        bindFunction(&Layout::makeBox, "make-box", "Creates a box based on text dimensions");
    }
};
}
}