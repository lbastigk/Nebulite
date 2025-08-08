#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite{
class JSON; // Forward declaration of container class JSON
namespace JSONTreeExpansion{


class SimpleData : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::JSON, SimpleData> {
public:
    using Wrapper<Nebulite::JSON, SimpleData>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE set(int argc, char* argv[]);
    Nebulite::ERROR_TYPE move(int argc, char* argv[]);
    Nebulite::ERROR_TYPE copy(int argc, char* argv[]);
    Nebulite::ERROR_TYPE keyDelete(int argc, char* argv[]);
    Nebulite::ERROR_TYPE push_back(int argc, char* argv[]);
    Nebulite::ERROR_TYPE pop_back(int argc, char* argv[]);
    Nebulite::ERROR_TYPE push_front(int argc, char* argv[]);
    Nebulite::ERROR_TYPE pop_front(int argc, char* argv[]);

    // TODO: Helper function 


    //----------------------------------------
    // Binding Functions
    void setupBindings()  {
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set, "set", "Set a key to a value in the JSON document");

        // Internal move/copy
        bindFunction(&SimpleData::move, "move", "Handles move calls");
        bindFunction(&SimpleData::copy, "copy", "Handles copy calls");

        // TODO: Interaction with global and potentially invoke for docCache?

        // Internal key deletion
        bindFunction(&SimpleData::keyDelete, "keyDelete", "Handles keyDelete calls");

        // Array manipulation
        bindFunction(&SimpleData::push_back,  "push-back",  "Pushes a value to the back of an array:  <key> <value>");
        bindFunction(&SimpleData::pop_back,   "pop-back",   "Pops a value from the back of an array:  <key>");
        bindFunction(&SimpleData::push_front, "push-front", "Pushes a value to the front of an array: <key> <value>");
        bindFunction(&SimpleData::pop_front,  "pop-front",  "Pops a value from the front of an array: <key>");
    }
};
}
}