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

namespace Nebulite::Core {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::DomainModule::JsonScope {
NEBULITE_DOMAINMODULE(Nebulite::Core::JsonScope, Debug) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error print(int argc, char** argv);
    static std::string_view constexpr print_name = "print";
    static std::string_view constexpr print_desc = "Prints the JSON document to the console for debugging purposes.\n"
        "If key is empty, prints the entire document.\n"
        "\n"
        "Usage: print [key]\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::JsonScope, Debug) {
        // Binding
        BINDFUNCTION(&Debug::print, print_name, print_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::DomainModule::JsonScope
#endif // NEBULITE_JSDM_DEBUG_HPP
