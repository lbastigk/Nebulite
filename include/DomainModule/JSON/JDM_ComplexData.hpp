/**
 * @file JDM_ComplexData.hpp
 * @brief DomainModule for complex data operations on domain class Nebulite::Utility::JSON
 */

#pragma once

//------------------------------------------
// Includes

// General
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

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
NEBULITE_DOMAINMODULE(Nebulite::Utility::JSON, ComplexData) {
public:
    /**
     * @brief Overwridden update function.
     */
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
    Nebulite::Constants::Error set_from_query(int argc, char* argv[]);

    /**
     * @brief Sets a key from a JSON document.
     * 
     * @param argc The argument count.
     * @param argv The argument vector: <key> <link:key>
     * @return Potential errors that occurred on command execution.
     * 
     * @todo Once all domains have access to the global space, 
     * use the JSON doc cache instead of loading the document each time.
     */
    Nebulite::Constants::Error set_from_json(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, ComplexData){
        // Bind functions specific to complex data handling

        // SQL Querys
        bindSubtree("query","Functions to manipulate JSON data via SQL query results");
        bindFunction(&ComplexData::set_from_query, "query set", "Sets a key from a SQL query result: <key> <query>");

        // Set from read only jsons
        bindSubtree("json","Functions to manipulate JSON data via JSON documents");
        bindFunction(&ComplexData::set_from_json,  "json set",  "Sets a key from a JSON document:    <key> <link:key>");
    }
};
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite