/**
 * @file ComplexData.hpp
 * @brief DomainModule for complex data operations on domain class Nebulite::Data::JSON
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
namespace Nebulite::Data {
class JSON;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::DomainModule::JSON {
/**
 * @class Nebulite::DomainModule::JSON::ComplexData
 * @brief DomainModule for complex data operations on domain class Nebulite::Data::JSON
 */
NEBULITE_DOMAINMODULE(Nebulite::Data::JSON, ComplexData) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @todo Not implemented yet
     */
    Constants::Error querySet(int argc, char** argv);
    static std::string_view constexpr querySet_name = "query set";
    static std::string_view constexpr querySet_desc = "Sets a key from a SQL query result.\n"
        "Not implemented yet.";

    Constants::Error jsonSet(int argc, char** argv);
    static std::string_view constexpr jsonSet_name = "json set";
    static std::string_view constexpr jsonSet_desc = "Sets a key from a read-only JSON document.\n"
        "Usage: json set <key> <link:key>\n"
        "\n"
        "Where <link:key> is a link to a JSON document.\n"
        "The document is dynamically loaded and cached for future use.";

    //------------------------------------------
    // Category names

    static std::string_view constexpr query_name = "query";
    static std::string_view constexpr query_desc = "Functions to manipulate JSON data via SQL query results";

    static std::string_view constexpr json_name = "json";
    static std::string_view constexpr json_desc = "Functions to manipulate JSON data via read-only JSON documents";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Data::JSON, ComplexData) {
        // Bind functions specific to complex data handling

        // SQL Queries
        (void)bindCategory(query_name, query_desc);
        bindFunction(&ComplexData::querySet, querySet_name, querySet_desc);

        // Set from read only JSON documents
        (void)bindCategory(json_name, json_desc);
        bindFunction(&ComplexData::jsonSet, jsonSet_name, jsonSet_desc);
    }
};
} // namespace Nebulite::JSON::DomainModule
#endif // NEBULITE_JSDM_COMPLEX_DATA_HPP
