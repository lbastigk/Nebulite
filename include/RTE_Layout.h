#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration of Container class RenderObject

namespace RenderObjectTreeExpansion {
class Layout : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, Layout> {
public:
    using Wrapper<Nebulite::RenderObject, Layout>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE alignText(int argc, char* argv[]);
    Nebulite::ERROR_TYPE makeBox(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions

    void setupBindings()  {
        bindFunction(&Layout::alignText, "align-text", "Aligns text to object dimensions");
        bindFunction(&Layout::makeBox, "make-box", "Creates a box based on text dimensions");
    }
};
}
}