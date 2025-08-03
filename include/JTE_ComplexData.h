#pragma once

#include "ErrorTypes.h"
#include "JT_ExpansionWrapper.h"

namespace Nebulite{
namespace JSONTreeExpansion{

class ComplexData : public Wrapper<ComplexData> {
public:
    using Wrapper<ComplexData>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE sqlCall(int argc, char* argv[]);
    Nebulite::ERROR_TYPE jsonCall(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        // Bind functions specific to complex data handling
        bindFunction(&ComplexData::sqlCall, "sqlCall", "Handles SQL calls");
        bindFunction(&ComplexData::jsonCall, "jsonCall", "Handles JSON calls");
    }
};
}
}