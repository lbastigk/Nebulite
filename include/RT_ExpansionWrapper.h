/*
This wrapper class extends the functionality of a general FuncTreeWrapper for the RenderObjectTree.
by providing a specific implementation for category-related function bindings.

This allows for cleaner separation of object files for different categories
and reduces boilerplate code when attaching functions to the FuncTree.

Within the Core FuncTree, Categories are initialized with references to the funcTree
and the RenderObject, allowing for easy access.

*/

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"


namespace Nebulite{

class RenderObject; // Forward declaration

namespace RenderObjectTreeExpansion{

template<typename DerivedClass>
class Wrapper{
public:
    // Binds all functions for this category on construction
    Wrapper(RenderObject* self, FuncTree<ERROR_TYPE>* funcTreePtr)
        : self(self), funcTree(funcTreePtr) 
    {
        static_cast<DerivedClass*>(this)->setupBindings();
    }

    // Binding helper
    void bindFunction(
            ERROR_TYPE (DerivedClass::*method)(int, char**),
            const std::string& name,
            const std::string& help) {
        funcTree->attachFunction(
            [this, method](int argc, char** argv) {
                return (static_cast<DerivedClass*>(this)->*method)(argc, argv);
            },
            name,
            help
        );
    }

    // Prevent copying
    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;

protected:
    //--------------------------
    // Linkages
    RenderObject* self;
    FuncTree<ERROR_TYPE>* funcTree;
};

}
}