#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite{
class JSON; // Forward declaration of domain class JSON
namespace JSONTreeExpansion{


class SimpleData : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::JSON, SimpleData> {
public:
    using Wrapper<Nebulite::JSON, SimpleData>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

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

    // Helper function to turn value into type array
    Nebulite::ERROR_TYPE ensureArray(int argc, char* argv[]);


    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set, "set", "Set a key to a value in the JSON document: <key> <value>");

        // Internal move/copy
        bindFunction(&SimpleData::move, "move", "Move data from one key to another: <source_key> <destination_key>");
        bindFunction(&SimpleData::copy, "copy", "Copy data from one key to another: <source_key> <destination_key>");

        // Internal key deletion
        bindFunction(&SimpleData::keyDelete, "keyDelete", "Delete a key from the JSON document: <key>");

        // Array manipulation
        bindFunction(&SimpleData::push_back,   "push-back",    "Pushes a value to the back of an array:  <key> <value>");
        bindFunction(&SimpleData::pop_back,    "pop-back",     "Pops a value from the back of an array:  <key>");
        bindFunction(&SimpleData::push_front,  "push-front",   "Pushes a value to the front of an array: <key> <value>");
        bindFunction(&SimpleData::pop_front,   "pop-front",    "Pops a value from the front of an array: <key>");
        bindFunction(&SimpleData::ensureArray, "ensure-array", "Ensures that a key is an array, converting a value to an array if necessary: <key>");   // Note: name 'ensure-array' is hardcoded in some functions! Do not change!
    }
};
}   // namespace JSONTreeExpansion
}   // namespace Nebulite