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
     * @brief Override of update.
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
    Nebulite::Constants::Error query_set(int argc, char* argv[]);
    static const std::string query_set_name;
    static const std::string query_set_desc;

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
    Nebulite::Constants::Error json_set(int argc, char* argv[]);
    static const std::string json_set_name;
    static const std::string json_set_desc;

    //------------------------------------------
    // Subtree names
    static const std::string query_name;
    static const std::string query_desc;

    static const std::string json_name;
    static const std::string json_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, ComplexData){
        // Bind functions specific to complex data handling

        // SQL Querys
        bindSubtree(query_name, &query_desc);
        bindFunction(&ComplexData::query_set, query_set_name, &query_set_desc);

        // Set from read only jsons
        bindSubtree(json_name, &json_desc);
        bindFunction(&ComplexData::json_set,  json_set_name,  &json_set_desc);
    }
};
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite