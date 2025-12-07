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
    void message(Context const& context);
    static constexpr std::string_view messageName = "::message";

    // Provides a simple debug message to cerr
    void error(Context const& context);
    static constexpr std::string_view errorName = "::error";

    //------------------------------------------
    // Constructor
    Debug() {
        // Bind Debug-related static rulesets here
        bind(RulesetType::Local, messageName, &Debug::message);
        bind(RulesetType::Local, errorName, &Debug::error);
    }
};
} // namespace Nebulite::Interaction::Rules::RulesetModules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULES_DEBUG_HPP