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

    static Constants::Error print(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr print_name = "print";
    static auto constexpr print_desc = "Prints the JSON document to the console for debugging purposes.\n"
        "If key is empty, prints the entire document.\n"
        "\n"
        "Usage: print [key]\n";

    static Constants::Error printId(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr printId_name = "print-id";
    static auto constexpr printId_desc = "Prints the unique ID of the domain to the console for debugging purposes.\n"
       "Usage: print-id\n";

    static Constants::Error error(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope);
    static auto constexpr error_name = "error";
    static auto constexpr error_desc = "Echoes all arguments as string to the standard error.\n"
        "Usage: error <string...>\n"
        "\n"
        "- <string...>: One or more strings to echo to the standard error.\n";

    static Constants::Error warn(std::span<std::string const> const& args);
    static auto constexpr warn_name = "warn";
    static auto constexpr warn_desc = "Returns a warning: a custom, noncritical error.\n"
        "Usage: warn <string>\n"
        "\n"
        "- <string>: The warning message.\n";

    static Constants::Error critical(std::span<std::string const> const& args);
    static auto constexpr critical_name = "critical";
    static auto constexpr critical_desc = "Returns a critical error.\n"
        "Usage: critical <string>\n"
        "\n"
        "- <string>: The critical error message.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Interaction::Execution::Domain, Debug) {
        // Binding
        BIND_FUNCTION(&Debug::print, print_name, print_desc);
        BIND_FUNCTION(&Debug::printId, printId_name, printId_desc);
        BIND_FUNCTION(&Debug::error, error_name, error_desc);
        BIND_FUNCTION(&Debug::warn, warn_name, warn_desc);
        BIND_FUNCTION(&Debug::critical, critical_name, critical_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::DomainModule::Common
#endif // NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP
