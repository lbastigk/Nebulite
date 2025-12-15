/**
*  @file Debug.hpp
 * @brief Debugging ruleset module for Nebulite interaction system.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_MODULES_DEBUG_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_MODULES_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library

// External

// Nebulite
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
// Forward declarations
namespace Nebulite::Interaction::Rules {
struct Context;
} // namespace Interaction::Rules

//------------------------------------------
namespace Nebulite::Interaction::Rules::RulesetModules {
class Debug : public RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Provides a simple debug message to cout
    void message(ContextBase const& context);
    static constexpr std::string_view messageName = "::message";
    static constexpr std::string_view messageDesc = "Outputs a debug message to the standard output (cout).";

    // Provides a simple debug message to cerr
    void error(ContextBase const& context);
    static constexpr std::string_view errorName = "::error";
    static constexpr std::string_view errorDesc = "Outputs a debug error message to the standard error output (cerr).";

    // Prints the ids of both contexts
    void who(ContextBase const& context);
    static constexpr std::string_view whoName = "::who";
    static constexpr std::string_view whoDesc = "Prints a message with the unique IDs of the self and other render object contexts.";

    //------------------------------------------
    // Constructor
    Debug() {
        // Local
        BIND_STATIC_ASSERT(RulesetType::Local, &Debug::message, messageName, messageDesc);
        BIND_STATIC_ASSERT(RulesetType::Local, &Debug::error, errorName, errorDesc);

        // Global
        BIND_STATIC_ASSERT(RulesetType::Global, &Debug::who, whoName, whoDesc);
    }
};
} // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_DEBUG_HPP