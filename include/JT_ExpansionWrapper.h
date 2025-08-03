/*
This wrapper class extends the functionality of a general FuncTreeWrapper for the JSONTree
by providing a specific implementation for category-related function bindings.

This allows for cleaner separation of object files for different categories
and reduces boilerplate code when attaching functions to the FuncTree.

Within the Core FuncTree, Categories are initialized with references to the funcTree
allowing them to individually bind functions on construction.

*/

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"


namespace Nebulite{

class JSON; // Forward declaration

namespace JSONTreeExpansion{

template<typename DerivedClass>
class Wrapper{
public:
    // Binds all functions for this category on construction
    Wrapper(JSON* self, FuncTree<ERROR_TYPE>* funcTreePtr)
        : funcTree(funcTreePtr), self(self)
    {
        // Initialize the defined Variable and Function Bindings
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

    // Attach a variable as string pointer to the FuncTree
    void bindVariable(std::string* varPtr, const std::string& name, const std::string& helpDescription) {
        funcTree->attachVariable(varPtr, name, helpDescription);
    }

    // Prevent copying
    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;

protected:
    //--------------------------
    // Linkages
    FuncTree<ERROR_TYPE>* funcTree;
    JSON* self;
};
}
}