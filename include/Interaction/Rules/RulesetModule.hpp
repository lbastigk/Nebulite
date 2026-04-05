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
#include "Data/OrderedCacheList.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

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
     * @brief Helper consteval function to determine if a string_view starts with '::' and has no whitespaces
     * @param str The string_view to check
     * @return true if str is a valid topic name, false otherwise
     */
    static consteval bool isValidTopic(std::string_view const& str) {
        return str.starts_with("::") && !str.contains(' ');
    }

    /**
     * @brief helper function to add a static ruleset to this module
     * @tparam topic The topic/name of the ruleset
     * @tparam DerivedRulesetModule The derived RulesetModule type
     * @param type The type of the ruleset (Local/Global)
     * @param func The function implementing the ruleset
     * @param description A brief description of the ruleset's purpose and its used variables
     * @param baseListFunc A function that returns the ordered cache list of base values required by this ruleset, given a context.
     * @todo Add an argument param std::span<std::string> const& args, so that we can have rulesets with arguments such as
     *       ::Controls::PT1 path.to.pt1.object
     *       topic must reduce to the first arg, and we must add the args to the static ruleset object
     */
    template<std::string_view const& topic, typename DerivedRulesetModule>
    void bind(
        RulesetType const& type,
        void (DerivedRulesetModule::*func)(Context const&, double**&, double**&) const,
        std::string_view const& description,
        BaseListFunction baseListFunc
    ){
        assert(func != nullptr);
        static_assert(isValidTopic(topic), "RulesetModule::bind(): The topic name is not valid. It must start with '::' and contain no spaces.");
        static_assert(std::is_base_of_v<RulesetModule, DerivedRulesetModule>, "RulesetModule::bind(): T must derive from RulesetModule");
        static_assert(std::is_same_v<decltype(DerivedRulesetModule::moduleName), const std::string_view>, "RulesetModule::bind(): DerivedRulesetModule must have a static member 'moduleName' of type std::string_view");
        static_assert(topic.starts_with(DerivedRulesetModule::moduleName), "RulesetModule::bind(): The topic name must start with the module's name as prefix.");
        moduleRulesets.push_back({
            type,
            topic,
            description,
            [this, func](Context const& ctx, double**& slf, double**& otr) { (static_cast<DerivedRulesetModule const*>(this)->*func)(ctx, slf, otr); },
            baseListFunc
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
        assert(v != nullptr);
        // NOLINTNEXTLINE
        return *v[static_cast<std::size_t>(k)];
    }

    /**
     * @brief Retrieves the ordered cache list of base values for the domain.
     *        Instead of retrieving each value individually, this function fetches all required values in a single call.
     *        This reduces lookup overhead and improves performance when accessing multiple base values.
     * @param domain The domain from which to retrieve the base values.
     * @param keys The array of keys to retrieve values for.
     * @param arr The output array of ordered cache lists corresponding to the provided keys.
     *            If arr passed is not null, this function will do nothing.
     */
    void ensureBaseList(Execution::Domain const& domain, std::vector<Data::ScopedKeyView> const& keys, double**& arr) const ;

    /**
     * @brief Generates a BaseList-ensurer function for any provided keys.
     * @param baseKeys The key list to retrieve
     * @return The BaseList-ensurer function.
     */
    std::function<double**(const Execution::Domain&)> generateBaseListFunction(std::vector<Data::ScopedKeyView> const& baseKeys) const ;

private:
    // Vector of all static rulesets from this module
    std::vector<StaticRulesetMap::StaticRuleSetWithMetaData> moduleRulesets;

    // Unique identifier for caching
    uint64_t const id;
};
} // namespace Nebulite::Interaction::Rules
#endif // NEBULITE_INTERACTION_RULES_RULESET_MODULE_HPP
