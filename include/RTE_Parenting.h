#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite {
class RenderObject; // Forward declaration of Domain class RenderObject

namespace RenderObjectTreeExpansion {
class Parenting : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::RenderObject, Parenting> {
public:
    using Wrapper<Nebulite::RenderObject, Parenting>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE addChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeChildren(int argc, char* argv[]);
    Nebulite::ERROR_TYPE removeAllChildren(int argc, char* argv[]);


    //----------------------------------------
    // Binding Functions

    void setupBindings()  {
        bindFunction(&Parenting::addChildren, "add-children", "Adds children of the RenderObject by name");
        bindFunction(&Parenting::removeChildren, "remove-children", "Removes children from the RenderObject");
        bindFunction(&Parenting::removeAllChildren, "remove-all-children", "Removes all children from the RenderObject");
    }
};
}
}