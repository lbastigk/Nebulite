/**
 * @file Debug.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_JSDM_DEBUG_HPP
#define NEBULITE_JSDM_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Utility {
class JSON;
} // namespace Nebulite::Utility

//------------------------------------------
namespace Nebulite::DomainModule::JSON {
NEBULITE_DOMAINMODULE(Nebulite::Utility::JSON, Debug) {
public:
    /**
     * @brief Override of update.
     */
    Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Prints the JSON document to the console for debugging purposes
     * 
     * If key is empty, prints the entire document.
     * 
     * @param argc The argument count
     * @param argv The argument vector: [key]
     * @return Potential errors that occurred on command execution
     */
    Constants::Error print(int argc, char** argv);
    static std::string const print_name;
    static std::string const print_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, Debug) {
        // Binding
        bindFunction(&Debug::print, print_name, &print_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::DomainModule::JSON
#endif // NEBULITE_JSDM_DEBUG_HPP