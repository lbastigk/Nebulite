/**
 * @file JDM_ComplexData.h
 * @brief DomainModule for complex data operations on domain class Nebulite::Utility::JSON
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.h"
#include "Interaction/Execution/DomainModuleWrapper.h"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Utility{
        class JSON; // Forward declaration of domain class JSON
    }
}

//------------------------------------------
namespace Nebulite{
namespace DomainModule{
namespace JSON{
/**
 * @class Nebulite::DomainModule::JSON::ComplexData
 * 
 * DomainModule for complex data operations on domain class Nebulite::Utility::JSON
 */
class ComplexData : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Utility::JSON, ComplexData> {
public:
    using DomainModuleWrapper<Nebulite::Utility::JSON, ComplexData>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    void update();

    //------------------------------------------
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
    Nebulite::Constants::ERROR_TYPE set_from_query(int argc, char* argv[]);

    /**
     * @brief Sets a key from a JSON document.
     * 
     * @param argc The argument count.
     * @param argv The argument vector: <key> <link:key>
     * @return Potential errors that occurred on command execution.
     * 
     * @todo Not implemented yet
     */
    Nebulite::Constants::ERROR_TYPE set_from_json(int argc, char* argv[]);

    //------------------------------------------
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
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite