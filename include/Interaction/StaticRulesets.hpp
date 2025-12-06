/**
 * @file StaticRulesets.hpp
 * @brief This file contains predefined static rulesets for common interactions in the Nebulite engine.
 *        Compared to json-defined rulesets, static rulesets are hardcoded for performance and reliability.
 */

#ifndef NEBULITE_INTERACTION_STATIC_RULESETS_HPP
#define NEBULITE_INTERACTION_STATIC_RULESETS_HPP

//------------------------------------------
// Includes

// Standard library

// External
#include "absl/container/flat_hash_map.h"

// Nebulite

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
class GlobalSpace;
}   // namespace Nebulite::Core

namespace Nebulite::Interaction::Execution {
class DomainBase;
}   // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction {

//------------------------------------------
// Defining what a ruleset function looks like

struct CoreContext {
    Nebulite::Interaction::Execution::DomainBase& self;
    Nebulite::Interaction::Execution::DomainBase& other;
    Nebulite::Interaction::Execution::DomainBase& global;
    // TODO: Parent context?
};

struct Context {
    Nebulite::Core::RenderObject& self;
    Nebulite::Core::RenderObject& other;
    Nebulite::Core::GlobalSpace& global;
    // TODO: Parent context?
};

// Basically: foo(context)
// Using basic function pointer syntax for maximum compatibility
using StaticRulesetFunctionCore = void(*)(CoreContext const& context);
using StaticRulesetFunction = void(*)(Context const& context);


//------------------------------------------
// Defining a Ruleset Map where static rulesets can be looked up by name

class StaticRulesetMap {
public:
    /**
     * @brief Retrieves a static ruleset function by name.
     * @param name The name of the static ruleset.
     * @return Pointer to the static ruleset function, or nullptr if not found.
     */
    static StaticRulesetFunction getStaticRulesetByName(std::string const& name);

    /**
     * @brief Adds a static ruleset function to the map.
     * @param name The name of the static ruleset.
     * @param func Pointer to the static ruleset function.
     */
    void bindStaticRuleset(std::string const& name, StaticRulesetFunction func) {
        // Exit program if duplicate
        if (container.contains(name)) {
            throw std::runtime_error("Duplicate static ruleset name: " + name);
        }
        container[name] = func;
    }

private:
    absl::flat_hash_map<std::string, StaticRulesetFunction> container;
};
} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_STATIC_RULESETS_HPP