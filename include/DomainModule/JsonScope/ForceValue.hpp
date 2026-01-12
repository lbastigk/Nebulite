/**
 * @file ForceValue.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef NEBULITE_JSDM_FORCE_VALUE_HPP
#define NEBULITE_JSDM_FORCE_VALUE_HPP

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
/**
 * @class Nebulite::DomainModule::JsonScope::ForceValue
 * @brief DomainModule for forcing JSON variable values.
 * @details This module allows users to force specific keys in the global JSON data
 *          to have specified values, overriding any existing values. It is useful for
 *          testing or overriding configuration values.
 */
NEBULITE_DOMAINMODULE(Nebulite::Core::JsonScope, ForceValue) {
public:
    Constants::Error update() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    Constants::Error force_set(int argc, char** argv);
    static auto constexpr force_set_name = "force set";
    static auto constexpr force_set_desc = "Force a variable to a value.\n"
        "\n"
        "Usage: force set <key> <value>\n";

    Constants::Error force_clear();
    static auto constexpr force_clear_name = "force clear";
    static auto constexpr force_clear_desc = "Clear all forced variables.\n"
        "\n"
        "Usage: force clear\n";

    //------------------------------------------
    // Category names
    static auto constexpr force_name = "force";
    static auto constexpr force_desc = "Category for forcing variables to specific values.\n"
        "This is useful for testing or overriding configuration values.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    NEBULITE_DOMAINMODULE_CONSTRUCTOR(Nebulite::Core::JsonScope, ForceValue){
        // Binding
        (void)bindCategory(force_name, force_desc);
        BINDFUNCTION(&ForceValue::force_set, force_set_name, force_set_desc);
        BINDFUNCTION(&ForceValue::force_clear, force_clear_name, force_clear_desc);
    }

private:
    absl::flat_hash_map<std::string, std::string> forced_global_values; // Key-Value pairs to set in global JSON
};
} // namespace Nebulite::DomainModule::JsonScope
#endif // NEBULITE_JSDM_FORCE_VALUE_HPP
