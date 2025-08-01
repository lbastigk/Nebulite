/*
This wrapper class extends the functionality of a general FuncTreeWrapper for the MainTree
by providing a specific implementation for category-related function bindings.

This allows for cleaner separation of object files for different categories
and reduces boilerplate code when attaching functions to the FuncTree.

Within the Core FuncTree, Categories are initialized with references to the funcTree
and the GlobalSpace, allowing them to individually bind functions on construction.

*/

#pragma once

#include "ErrorTypes.h"
#include "FuncTreeWrapper.h"
#include "Invoke.h"


namespace Nebulite{

class GlobalSpace; // Forward declaration

namespace MainTreeExpansion{

template<typename DerivedClass>
class Wrapper{
public:
    // Binds all functions for this category on construction
    Wrapper(Invoke* invoke, GlobalSpace* globalSpace, FuncTree<ERROR_TYPE>* funcTreePtr)
        : invoke(invoke), global(globalSpace), funcTree(funcTreePtr) 
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
    Invoke* invoke;
    GlobalSpace* global;
    FuncTree<ERROR_TYPE>* funcTree;
};
}
}