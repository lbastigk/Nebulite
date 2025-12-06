/**
 * @file RulesetModule.hpp
 * @brief This file defines the RulesetModule class, for defining classes of static rulesets.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP

//------------------------------------------
// Includes

// Standard library
#include <vector>
#include <string>
#include <type_traits>

// External

// Nebulite
#include "Interaction/Rules/StaticRulesets.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules {
class RulesetModule {
public:
    using RulesetType = StaticRulesetMap::StaticRuleSetWithMetaData::Type;

    /**
     * @brief helper function to add a static ruleset to this module
     * @param type The type of the ruleset (Local/Global)
     * @param topic The topic/name of the ruleset
     * @param func The function implementing the ruleset
     */
    template<typename T>
    void bind(RulesetType const& type, std::string const& topic, void (T::*func)(Context const&)) {
        static_assert(std::is_base_of_v<RulesetModule, T>, "bind(): T must derive from RulesetModule");
        moduleRulesets.push_back({
            type,
            topic,
            [this, func](Context const& ctx) { (static_cast<T*>(this)->*func)(ctx); }
        });
    }

    /**
     * @brief Registers all static rulesets from this module into the given container
     * @param container The StaticRulesetMap to register into
     */
    void registerModule(StaticRulesetMap& container) {
        for (auto const& ruleset : moduleRulesets) {
            container.bindStaticRuleset(ruleset.topic, ruleset);
        }
    }
private:
    // Vector of all static rulesets from this module
    std::vector<StaticRulesetMap::StaticRuleSetWithMetaData> moduleRulesets;
};
}
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP