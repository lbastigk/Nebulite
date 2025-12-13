/**
 * @file RulesetModule.hpp
 * @brief This file defines the RulesetModule class, for defining classes of static rulesets.
 */

#ifndef NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP
#define NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>
#include <type_traits>
#include <vector>

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
     * @param description A brief description of the ruleset's purpose and its used variables
     * @todo Rework all implementations to make use of the description parameter
     */
    template<typename T>
    void bind(RulesetType const& type, std::string_view const& topic, void (T::*func)(Context const&), std::string const& description = "") {
        static_assert(std::is_base_of_v<RulesetModule, T>, "bind(): T must derive from RulesetModule");
        if (!topic.starts_with("::")) {
            throw std::invalid_argument("RulesetModule::bind(): topic must start with '::'. Tried to bind: " + std::string(topic));
        }
        moduleRulesets.push_back({
            type,
            std::string(topic),
            [this, func](Context const& ctx) { (static_cast<T*>(this)->*func)(ctx); },
            description
        });
    }

    /**
     * @brief Registers all static rulesets from this module into the given container
     * @param container The StaticRulesetMap to register into
     */
    void registerModule(StaticRulesetMap& container) {
        for (auto const& ruleset : moduleRulesets) {
            container.bindStaticRuleset(ruleset);
        }
    }

    /**
     * @brief Helper function to retrieve an ordered list of stable double pointers
     *        arrays of keys to arrays of values
     *        with a unique identifier for each array of values
     *        Use the function name itself "::<function>" as the unique identifier
     *        hash in globalspace to avoid collisions.
     * @param doc The document in which to retrieve the values
     * @param identifier The unique identifier for the array of values
     * @param keys The array of keys to retrieve values for
     * @return An array of values corresponding to the provided keys
     */
    static Data::odpvec* ensureOrderedCacheList(Nebulite::Data::JSON& doc, uint64_t const& identifier, std::vector<std::string> const& keys) {
        auto map = doc.getExpressionRefs();
        return map->ensureOrderedCacheList(identifier, &doc, keys);
    }
protected:
    // TODO: offer an interface for access to common variables here instead of re-implementing in each module
    //       perhaps making RulesetModule templated, taking its enum as a template parameter?
    //       template<typename CommonValues> class RulesetModule { ... }
    //       or just templating the getter function, should be sufficient

private:
    // Vector of all static rulesets from this module
    std::vector<StaticRulesetMap::StaticRuleSetWithMetaData> moduleRulesets;
};
}
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP