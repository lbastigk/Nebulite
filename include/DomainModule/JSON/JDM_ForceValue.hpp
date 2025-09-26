/**
 * @file JDM_ForceValue.hpp
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
NEBULITE_DOMAINMODULE(Nebulite::Utility::JSON, ForceValue) {
public:
    /**
     * @brief Overwridden update function.
     */
    void update();

    //------------------------------------------
    // Available Functions

    /**
     * @brief Forces a variable to a specific value
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error force(int argc, char* argv[]);

    /**
     * @brief Clears all forced variables
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error forceClear(int argc, char* argv[]);

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, ForceValue){
        // Binding
        bindSubtree("force", "Functions to force JSON variable values");
        bindFunction(&ForceValue::force,      "force set",      "Force a variable to a value: force-global <key> <value>");
        bindFunction(&ForceValue::forceClear, "force clear",    "Clear all forced variables");
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
}   // namespace JSON
}   // namespace DomainModule
}   // namespace Nebulite

