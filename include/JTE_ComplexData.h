#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite{
class JSON; // Forward declaration of container class JSON
namespace JSONTreeExpansion{

class ComplexData : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::JSON, ComplexData> {
public:
    using Wrapper<Nebulite::JSON, ComplexData>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //----------------------------------------
    // Available Functions
    Nebulite::ERROR_TYPE set_from_query(int argc, char* argv[]);
    Nebulite::ERROR_TYPE set_from_json(int argc, char* argv[]);

    //----------------------------------------
    // Binding Functions
    void setupBindings() {
        // Bind functions specific to complex data handling
        bindFunction(&ComplexData::set_from_query, "set-from-query", "Sets a key from a SQL query result: <key> <query>");
        bindFunction(&ComplexData::set_from_json,  "set-from-json",  "Sets a key from a JSON document:    <key> <link:key>");
    }
};
}
}