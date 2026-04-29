/**
 * @file Debug.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP
#define NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/StandardCapture.hpp"
#include "Interaction/Execution/DomainModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
NEBULITE_DOMAINMODULE(Nebulite::Interaction::Execution::Domain, Debug) {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    [[nodiscard]] static Constants::Event print(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr print_name = "print";
    static auto constexpr print_desc = "Prints the JSON document to the console for debugging purposes.\n"
        "If key is empty, prints the entire document.\n"
        "\n"
        "Usage: print [key]\n";

    [[nodiscard]] static Constants::Event printId(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr printId_name = "print-id";
    static auto constexpr printId_desc = "Prints the unique ID of the domain to the console for debugging purposes.\n"
       "Usage: print-id\n";

    [[nodiscard]] static Constants::Event error(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr error_name = "error";
    static auto constexpr error_desc = "Echoes all arguments as string to the standard error.\n"
        "Usage: error <string...>\n"
        "\n"
        "- <string...>: One or more strings to echo to the standard error.\n";

    [[nodiscard]] Constants::Event warn(std::span<std::string const> const& args) const ;
    static auto constexpr warn_name = "warn";
    static auto constexpr warn_desc = "Sends a warning to the capture.\n"
        "Usage: warn <string>\n"
        "\n"
        "- <string>: The warning message.\n";

    [[nodiscard]] Constants::Event critical(std::span<std::string const> const& args) const ;
    static auto constexpr critical_name = "critical";
    static auto constexpr critical_desc = "Sends an error to the capture.\n"
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
        bindFunction(&Debug::print, print_name, print_desc);
        bindFunction(&Debug::printId, printId_name, printId_desc);
        bindFunction(&Debug::error, error_name, error_desc);
        bindFunction(&Debug::warn, warn_name, warn_desc);
        bindFunction(&Debug::critical, critical_name, critical_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_DOMAIN_MODULE_COMMON_DEBUG_HPP
