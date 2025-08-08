/*
This wrapper class extends the functionality of a general FuncTreeWrapper for the GlobalSpaceTree
by providing a specific implementation for category-related function bindings.

This allows for cleaner separation of object files for different categories
and reduces boilerplate code when attaching functions to the FuncTree.

Within the Core FuncTree, Categories are initialized with references to the funcTree,
invokeand the GlobalSpace, allowing them to individually bind functions on construction.

*/

#pragma once

#include "ErrorTypes.h"
#include "FuncTree.h"


namespace Nebulite{

class GlobalSpace; // Forward declaration

namespace FuncTreeExpansion{

template<typename DomainType, typename DerivedClass>
class Wrapper{
public:
    // Binds all functions for this category on construction
    Wrapper(DomainType* domain, FuncTree<ERROR_TYPE>* funcTreePtr)
        : self(domain), funcTree(funcTreePtr) 
    {
        // Initialize the defined Variable and Function Bindings
        static_cast<DerivedClass*>(this)->setupBindings();
    }

    // A virtual function would be better so that we know that the derived class has to implement it
    // However, this would call a pure virtual function during construction, which is not allowed.
    //virtual void setupBindings() = 0; // Pure virtual function to be implemented by derived classes

    // Templated bindFunction overload - automatically handles any class type
    template<typename ClassType>
    void bindFunction(
        ERROR_TYPE (ClassType::*method)(int, char**),
        const std::string& name,
        const std::string& help) {
        // Automatically pass 'this' (the derived class instance) to bindFunction
        funcTree->bindFunction(
            static_cast<ClassType*>(this),  // Auto-cast to correct type
            method,                         // Member function pointer
            name, 
            help
        );
    }

    void bindVariable(
        std::string* variablePtr,
        const std::string& name,
        const std::string& help) {
        // Bind a variable to the FuncTree
        funcTree->bindVariable(variablePtr, name, help);
    }

    // Prevent copying
    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;

protected:
    //--------------------------
    // Linkages
    DomainType* self;                // Workspace of the expansion
    FuncTree<ERROR_TYPE>* funcTree;     // Where to bind the expanded functions
};
}
}