/**
 * @file JDM_Debug.hpp
 * 
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite{
    namespace Utility{
        class JSON; // Forward declaration of Utility::JSON
    }
}

//------------------------------------------
namespace Nebulite {
namespace DomainModule {
namespace JSON {
NEBULITE_DOMAINMODULE(Nebulite::Utility::JSON, Debug) {
public:
    /**
     * @brief Override of update.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Prints the JSON document to the console for debugging purposes
     * 
     * If key is empty, prints the entire document.
     * 
     * @param argc The argument count
     * @param argv The argument vector: [key]
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error print(int argc, char* argv[]);
    static const std::string print_name;
    static const std::string print_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, Debug){
        // Binding
        bindFunction(&Debug::print, print_name, &print_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
}   // namespace JSON
}   // namespace DomainModule
}   // namespace Nebulite

