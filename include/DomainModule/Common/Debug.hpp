/**
 * @file Debug.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP
#define NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/ErrorTypes.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
// Forward declarations

//------------------------------------------
namespace Nebulite::DomainModule::Common {
NEBULITE_DOMAINMODULE(Nebulite::Interaction::Execution::Domain, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    static Constants::Error print(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope);
    static auto constexpr print_name = "print";
    static auto constexpr print_desc = "Prints the JSON document to the console for debugging purposes.\n"
        "If key is empty, prints the entire document.\n"
        "\n"
        "Usage: print [key]\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Interaction::Execution::Domain, Debug) {
        // Binding
        BIND_FUNCTION(&Debug::print, print_name, print_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::DomainModule::Common
#endif // NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP
