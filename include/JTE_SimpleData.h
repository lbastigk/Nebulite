#pragma once

#include "ErrorTypes.h"
#include "JT_ExpansionWrapper.h"

namespace Nebulite{
namespace JSONTreeExpansion{

class SimpleData : public Wrapper<SimpleData> {
public:
    using Wrapper<SimpleData>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE set(int argc, char* argv[]);
    Nebulite::ERROR_TYPE store(int argc, char* argv[]);
    Nebulite::ERROR_TYPE move(int argc, char* argv[]);
    Nebulite::ERROR_TYPE copy(int argc, char* argv[]);
    Nebulite::ERROR_TYPE keyDelete(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        // Bind functions specific to complex data handling
        bindFunction(&SimpleData::set, "set", "Handles set calls");
        bindFunction(&SimpleData::store, "store", "Handles store calls");
        bindFunction(&SimpleData::move, "move", "Handles move calls");
        bindFunction(&SimpleData::copy, "copy", "Handles copy calls");
        bindFunction(&SimpleData::keyDelete, "keyDelete", "Handles keyDelete calls");
    }
};
}
}