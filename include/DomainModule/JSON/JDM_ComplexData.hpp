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
class ComplexData : public Nebulite::Interaction::Execution::DomainModule<Nebulite::Utility::JSON> {
public:
    /**
     * @brief Overridden update function.
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
     * @brief Initializes references to the domain and FuncTree, 
     * and binds functions to the FuncTree.
     */
    ComplexData(std::string moduleName, Nebulite::Utility::JSON* domain, Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::Error>* funcTreePtr) 
    : DomainModule(moduleName, domain, funcTreePtr) {
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