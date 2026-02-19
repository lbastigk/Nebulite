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
#include "ScopeAccessor.hpp"
#include "Core/JsonScope.hpp"
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
    bind(type, func, topic, description)

//------------------------------------------
namespace Nebulite::Interaction::Rules {
class RulesetModule {
public:
    using RulesetType = StaticRulesetMap::StaticRuleSetWithMetaData::Type;

    explicit RulesetModule(std::string_view const& moduleName);

    /**
     * @brief Registers all static rulesets from this module into the given container
     * @param container The StaticRulesetMap to register into
     */
    void registerModule(StaticRulesetMap& container) const {
        for (auto const& ruleset : moduleRulesets) {
            container.bindStaticRuleset(ruleset);
        }
    }

protected:
    /**
     * @brief Helper function to get a RulesetModuleToken for a derived module
     * @param derivedModule The derived RulesetModule instance, used for the prefix
     * @return A RulesetModuleToken for the derived module
     */
    static ScopeAccessor::RulesetModuleToken getRulesetModuleAccessToken(RulesetModule const& derivedModule){
        return ScopeAccessor::RulesetModuleToken(derivedModule);
    }

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
     * @tparam T The derived RulesetModule type
     * @param type The type of the ruleset (Local/Global)
     * @param func The function implementing the ruleset
     * @param topic The topic/name of the ruleset
     * @param description A brief description of the ruleset's purpose and its used variables
     */
    template<typename T>
    void bind(RulesetType const& type, void (T::*func)(Context const&), std::string_view const& topic, std::string_view const& description){
        static_assert(std::is_base_of_v<RulesetModule, T>, "bind(): T must derive from RulesetModule");
        if (!topic.starts_with("::")) {
            throw std::invalid_argument("RulesetModule::bind(): topic must start with '::'. Tried to bind: " + std::string(topic));
        }
        moduleRulesets.push_back({
            type,
            topic,
            description,
            [this, func](Context const& ctx) { (static_cast<T*>(this)->*func)(ctx); }
        });
    }

    /**
     * @brief helper function to add a static ruleset to this module (const version)
     *        Use the BIND_STATIC_ASSERT macro instead to both check and bind in one line
     * @tparam T The derived RulesetModule type
     * @param type The type of the ruleset (Local/Global)
     * @param func The function implementing the ruleset
     * @param topic The topic/name of the ruleset
     * @param description A brief description of the ruleset's purpose and its used variables
     */
    template<typename T>
    void bind(RulesetType const& type, void (T::*func)(Context const&) const, std::string_view const& topic, std::string_view const& description){
        static_assert(std::is_base_of_v<RulesetModule, T>, "bind(): T must derive from RulesetModule");
        if (!topic.starts_with("::")) {
            throw std::invalid_argument("RulesetModule::bind(): topic must start with '::'. Tried to bind: " + std::string(topic));
        }
        moduleRulesets.push_back({
            type,
            topic,
            description,
            [this, func](Context const& ctx) { (static_cast<T const*>(this)->*func)(ctx); }
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
    static double& baseVal(double** v, keyEnum k) noexcept {
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
    [[nodiscard]] double** getBaseList(Execution::Domain const& ctx, std::vector<Data::ScopedKeyView> const& keys) const {
        return ctx.ensureOrderedCacheList(id, keys)->data();
    }

private:
    // Vector of all static rulesets from this module
    std::vector<StaticRulesetMap::StaticRuleSetWithMetaData> moduleRulesets;

    // Unique identifier for caching
    uint64_t const id;
};
}
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP
