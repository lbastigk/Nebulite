#pragma once

#include "ErrorTypes.h"
#include "RT_ExpansionWrapper.h"

namespace Nebulite {

// Forward declaration of classes
class Invoke;
class GlobalSpace;

namespace RenderObjectTreeExpansion {
class Data : public Wrapper<Data> {
public:
    using Wrapper<Data>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE store(int argc, char* argv[]);
    Nebulite::ERROR_TYPE move(int argc, char* argv[]);
    Nebulite::ERROR_TYPE copy(int argc, char* argv[]);
    Nebulite::ERROR_TYPE keyDelete(int argc, char* argv[]);
    Nebulite::ERROR_TYPE sqlCall(int argc, char* argv[]);
    Nebulite::ERROR_TYPE jsonCall(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions

    void setupBindings(){
        bindFunction(&Data::store, "store", "Stores object data");
        bindFunction(&Data::move, "move", "Moves object data");
        bindFunction(&Data::copy, "copy", "Copies object data");
        bindFunction(&Data::keyDelete, "key-delete", "Deletes object key");
        bindFunction(&Data::sqlCall, "sql-call", "Calls SQL function");
        bindFunction(&Data::jsonCall, "json-call", "Calls JSON function");
    }
};
}
}