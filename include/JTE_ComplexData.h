#pragma once

#include "ErrorTypes.h"
#include "FuncTreeExpansionWrapper.h"

namespace Nebulite{
class JSON; // Forward declaration of container class JSON
namespace JSONTreeExpansion{

class ComplexData : public Nebulite::FuncTreeExpansion::Wrapper<Nebulite::JSON, ComplexData> {
public:
    using Wrapper<Nebulite::JSON, ComplexData>::Wrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //----------------------------------------
    // Available Functions

    /**
     * @brief Sets a key from a SQL query result.
     * 
     * @param argc The argument count.
     * @param argv The argument vector: <key> <query>
     * @return Potential errors that occurred on command execution.
     * 
     * @todo Not implemented yet
     */
    Nebulite::ERROR_TYPE set_from_query(int argc, char* argv[]);

    /**
     * @brief Sets a key from a JSON document.
     * 
     * @param argc The argument count.
     * @param argv The argument vector: <key> <link:key>
     * @return Potential errors that occurred on command execution.
     * 
     * @todo Not implemented yet
     */
    Nebulite::ERROR_TYPE set_from_json(int argc, char* argv[]);

    //-------------------------------------------
    // Setup

    /**
     * @brief Sets up the functions bindings in the domains function tree
     * 
     * Is called automatically by the inherited Wrappers constructor.
     */
    void setupBindings() {
        // Bind functions specific to complex data handling
        bindFunction(&ComplexData::set_from_query, "set-from-query", "Sets a key from a SQL query result: <key> <query>");
        bindFunction(&ComplexData::set_from_json,  "set-from-json",  "Sets a key from a JSON document:    <key> <link:key>");
    }
};
}   // namespace JSONTreeExpansion
}   // namespace Nebulite