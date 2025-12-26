/**
 * @file ForceValue.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_JSDM_FORCEVALUE_HPP
#define NEBULITE_JSDM_FORCEVALUE_HPP

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
NEBULITE_DOMAINMODULE(Nebulite::Data::JSON, ForceValue) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    /**
     * @brief Forces a variable to a specific value
     * 
     * @param argc The argument count
     * @param argv The argument vector: <key> <newvalue>
     * @return Potential errors that occurred on command execution
     */
    Constants::Error force_set(int argc, char** argv);
    static std::string const force_set_name;
    static std::string const force_set_desc;

    /**
     * @brief Clears all forced variables
     * 
     * @param argc The argument count
     * @param argv The argument vector: No arguments available
     * @return Potential errors that occurred on command execution
     */
    Constants::Error force_clear(int argc, char** argv);
    static std::string const force_clear_name;
    static std::string const force_clear_desc;

    //------------------------------------------
    // Category names
    static std::string const force_name;
    static std::string const force_desc;

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Data::JSON, ForceValue)
    {
        // Binding
        (void)bindCategory(force_name, force_desc);
        bindFunction(&ForceValue::force_set, force_set_name, force_set_desc);
        bindFunction(&ForceValue::force_clear, force_clear_name, force_clear_desc);
    }

    private
    :
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::DomainModule::JSON
#endif // NEBULITE_JSDM_FORCEVALUE_HPP
