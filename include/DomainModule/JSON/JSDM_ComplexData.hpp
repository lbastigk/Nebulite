/**
 * @file JSDM_ComplexData.hpp
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
    Nebulite::Constants::Error update() override;

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
    Nebulite::Constants::Error query_set(int argc,  char* argv[]);
    static std::string const query_set_name;
    static std::string const query_set_desc;

    /**
     * @brief Sets a key from a JSON document.
     * 
     * @param argc The argument count.
     * @param argv The argument vector: <key> <link:key>
     * @return Potential errors that occurred on command execution.
     */
    Nebulite::Constants::Error json_set(int argc,  char* argv[]);
    static std::string const json_set_name;
    static std::string const json_set_desc;

    //------------------------------------------
    // Category names
    static std::string const query_name;
    static std::string const query_desc;

    static std::string const json_name;
    static std::string const json_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, ComplexData){
        // Bind functions specific to complex data handling

        // SQL Querys
        bindCategory(query_name, &query_desc);
        bindFunction(&ComplexData::query_set, query_set_name, &query_set_desc);

        // Set from read only jsons
        bindCategory(json_name, &json_desc);
        bindFunction(&ComplexData::json_set,  json_set_name,  &json_set_desc);
    }
};
}   // namespace DomainModule
}   // namespace JSON
}   // namespace Nebulite