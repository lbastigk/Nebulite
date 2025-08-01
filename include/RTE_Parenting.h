#pragma once

#include "ErrorTypes.h"
#include "RT_ExpansionWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace RenderObjectTreeExpansion {
class Parenting : public Wrapper<Parenting> {
public:
    using Wrapper<Parenting>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE addChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeAllChildren(int argc, char* argv[]);


    //----------------------------------------
    // Binding Functions

    void setupBindings(){
        bindFunction(&Parenting::addChildren, "add-children", "Adds children of the RenderObject by name");
        bindFunction(&Parenting::removeChildren, "remove-children", "Removes children from the RenderObject");
        bindFunction(&Parenting::removeAllChildren, "remove-all-children", "Removes all children from the RenderObject");
    }
};
}
}