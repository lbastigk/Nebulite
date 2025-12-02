/**
 * @file ComplexData.hpp
 * @brief DomainModule for complex data operations on domain class Nebulite::Utility::JSON
 */

#ifndef NEBULITE_JSDM_COMPLEX_DATA_HPP
#define NEBULITE_JSDM_COMPLEX_DATA_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Utility {
    class JSON;
}   // namespace Nebulite::Utility

//------------------------------------------
namespace Nebulite::DomainModule::JSON {
/**
 * @class Nebulite::DomainModule::JSON::ComplexData
 * 
 * DomainModule for complex data operations on domain class Nebulite::Utility::JSON
 */
NEBULITE_DOMAINMODULE(Nebulite::Utility::JSON, ComplexData){
public:
    /**
     * @brief Override of update.
     */
    Constants::Error update() override;

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
    Constants::Error querySet(int argc,  char** argv);
    static std::string const querySet_name;
    static std::string const querySet_desc;

    /**
     * @brief Sets a key from a JSON document.
     * 
     * @param argc The argument count.
     * @param argv The argument vector: <key> <link:key>
     * @return Potential errors that occurred on command execution.
     */
    Constants::Error jsonSet(int argc,  char** argv);
    static std::string const jsonSet_name;
    static std::string const jsonSet_desc;

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

        // SQL Queries
        (void)bindCategory(query_name, &query_desc);
        bindFunction(&ComplexData::querySet, querySet_name, &querySet_desc);

        // Set from read only jsons
        (void)bindCategory(json_name, &json_desc);
        bindFunction(&ComplexData::jsonSet,  jsonSet_name,  &jsonSet_desc);
    }
};
}   // namespace Nebulite::JSON::DomainModule
#endif // NEBULITE_JSDM_COMPLEX_DATA_HPP