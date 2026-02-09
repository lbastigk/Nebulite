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

namespace Nebulite::Core {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::DomainModule::JsonScope {
/**
 * @class Nebulite::DomainModule::JsonScope::ComplexData
 * @brief DomainModule for complex data operations on domain class Nebulite::Data::JSON
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::JsonScope, ComplexData) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @todo Not implemented yet
     */
    Constants::Error querySet();
    static auto constexpr querySet_name = "query set";
    static auto constexpr querySet_desc = "Sets a key from a SQL query result.\n"
        "Not implemented yet.\n";

    static Constants::Error jsonSet(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr jsonSet_name = "json set";
    static auto constexpr jsonSet_desc = "Sets a key from an expression evaluated as JSON, allowing for complex objects to be set.\n"
        "Usage: json set <key> <expression>\n"
        "\n"
        "Examples:\n"
        "json set namesStartingWithF {global.names|filterGlob F*}\n"
        "json set userInfo {global.users|filterRegex {!^user[0-9]+$}}\n"
        "json set readOnlyDoc {./Resources/sample.json:key1.key2}\n"
        "json set sizeCopy {self.size}\n";

    //------------------------------------------
    // Category names

    static auto constexpr query_name = "query";
    static auto constexpr query_desc = "Functions to manipulate JSON data via SQL query results";

    static auto constexpr json_name = "json";
    static auto constexpr json_desc = "Functions to manipulate JSON data via read-only JSON documents";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::JsonScope, ComplexData) {
        // Bind functions specific to complex data handling

        // SQL Queries
        bindCategory(query_name, query_desc);
        BIND_FUNCTION(&ComplexData::querySet, querySet_name, querySet_desc);

        // Set from read only JSON documents
        bindCategory(json_name, json_desc);
        BIND_FUNCTION(&ComplexData::jsonSet, jsonSet_name, jsonSet_desc);
    }
};
} // namespace Nebulite::JSON::DomainModule
#endif // NEBULITE_JSDM_COMPLEX_DATA_HPP
