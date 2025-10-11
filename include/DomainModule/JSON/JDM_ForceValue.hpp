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
     * @brief Override of update.
     */
    Nebulite::Constants::Error update() override;

    //------------------------------------------
    // Available Functions

    /**
     * @brief Forces a variable to a specific value
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error force_set(int argc, char* argv[]);
    static const std::string force_set_name;
    static const std::string force_set_desc;

    /**
     * @brief Clears all forced variables
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::Error force_clear(int argc, char* argv[]);
    static const std::string force_clear_name;
    static const std::string force_clear_desc;

    //------------------------------------------
    // Subtree names
    static const std::string force_name;
    static const std::string force_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Utility::JSON, ForceValue){
        // Binding
        bindSubtree(force_name, &force_desc);
        bindFunction(&ForceValue::force_set,      force_set_name,      &force_set_desc);
        bindFunction(&ForceValue::force_clear,    force_clear_name,    &force_clear_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
}   // namespace JSON
}   // namespace DomainModule
}   // namespace Nebulite

