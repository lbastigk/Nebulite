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
#include "Data/JSON.hpp"
#include "Data/OrderedDoublePointers.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

//------------------------------------------

/**
 * @brief Macro to bind a static ruleset with a compile-time assertion on the topic format
 * @param type The type of the ruleset (Local/Global)
 * @param func The function implementing the ruleset
 * @param topic The topic/name of the ruleset
 * @param description A brief description of the ruleset's purpose and its used variables
 */
#define BIND_STATIC_ASSERT(type, func, topic, description) \
    static_assert(Nebulite::Interaction::Rules::RulesetModule::isValidTopic(topic), \
    "BIND_STATIC_ASSERT(): A static rulesets topic must start with '::'. Tried to bind variable: " #topic); \
    bind(type, func, topic, description);

//------------------------------------------
namespace Nebulite::Interaction::Rules {
class RulesetModule {
public:
    using RulesetType = StaticRulesetMap::StaticRuleSetWithMetaData::Type;

    RulesetModule(std::string_view const& moduleName);

    /**
     * @brief Registers all static rulesets from this module into the given container
     * @param container The StaticRulesetMap to register into
     */
    void registerModule(StaticRulesetMap& container) {
        for (auto const& ruleset : moduleRulesets) {
            container.bindStaticRuleset(ruleset);
        }
    }

protected:
    /**
     * @brief Helper consteval function to determine if a string_view starts with '::'
     * @param str The string_view to check
     * @return true if str starts with '::', false otherwise
     */
    static consteval bool isValidTopic(std::string_view const& str) {
        return str.starts_with("::");
    }

    /**
     * @brief helper function to add a static ruleset to this module
     *        Use the BIND_STATIC_ASSERT macro instead to both check and bind in one line
     * @param type The type of the ruleset (Local/Global)
     * @param func The function implementing the ruleset
     * @param topic The topic/name of the ruleset
     * @param description A brief description of the ruleset's purpose and its used variables
     */
    template<typename T>
    void bind(RulesetType const& type, void (T::*func)(ContextBase const&), std::string_view const& topic, std::string_view const& description) {
        static_assert(std::is_base_of_v<RulesetModule, T>, "bind(): T must derive from RulesetModule");
        if (!topic.starts_with("::")) {
            throw std::invalid_argument("RulesetModule::bind(): topic must start with '::'. Tried to bind: " + std::string(topic));
        }
        moduleRulesets.push_back({
            type,
            topic,
            description,
            [this, func](ContextBase const& ctx) { (static_cast<T*>(this)->*func)(ctx); }
        });
    }

    //------------------------------------------
    // Ordered cache list retrieval for base values

    /**
     * @brief Retrieves a base value from the ordered cache list for the given key.
     * @param v The ordered cache list of base values.
     * @param k The key corresponding to the desired base value.
     * @return A reference to the base value associated with the specified key.
     * @tparam keyEnum An enumeration type representing the keys for base values.
     */
    template<typename keyEnum>
    inline static double& baseVal(double** v, keyEnum k) noexcept {
        return *v[static_cast<std::size_t>(k)];
    }

    /**
     * @brief Retrieves the ordered cache list of base values for the given render object context.
     *        Instead of retrieving each value individually, this function fetches all required values in a single call.
     *        This reduces lookup overhead and improves performance when accessing multiple base values.
     * @param ctx The render object context from which to retrieve the base values.
     * @param keys The array of keys to retrieve values for.
     * @return A pointer to an array of double pointers, each pointing to a base value.
     */
    double** getBaseList(Interaction::Execution::DomainBase& ctx, std::vector<std::string> const& keys) {
        return ensureOrderedCacheList(*ctx.getDoc(), keys)->data();
    }

private:
    // Vector of all static rulesets from this module
    std::vector<StaticRulesetMap::StaticRuleSetWithMetaData> moduleRulesets;

    // Unique identifier for caching
    uint64_t const id;

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
    Data::odpvec* ensureOrderedCacheList(Nebulite::Data::JSON& doc, std::vector<std::string> const& keys) const {
        return doc.getOrderedCacheListMap()->ensureOrderedCacheList(id, &doc, keys);
    }
};
}
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP