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
#include "Interaction/Execution/DomainModuleWrapper.hpp"

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
class ForceValue : public Nebulite::Interaction::Execution::DomainModuleWrapper<Nebulite::Utility::JSON, ForceValue> {
public:
    using DomainModuleWrapper<Nebulite::Utility::JSON, ForceValue>::DomainModuleWrapper; // Templated constructor from Wrapper, call this->setupBindings()

    //------------------------------------------
    // Available Functions

    /**
     * @brief Forces a variable to a specific value
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE force(int argc, char* argv[]);

    /**
     * @brief Clears all forced variables
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <value>
     * @return Potential errors that occured on command execution
     */
    Nebulite::Constants::ERROR_TYPE forceClear(int argc, char* argv[]);

    //------------------------------------------
    // Setup
    void setupBindings() {
        // Binding
        bindFunction(&ForceValue::force,      "force",         "Force a variable to a value: force-global <key> <value>");
        bindFunction(&ForceValue::forceClear, "force-clear",   "Clear all forced variables");
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
}   // namespace JSON
}   // namespace DomainModule
}   // namespace Nebulite

