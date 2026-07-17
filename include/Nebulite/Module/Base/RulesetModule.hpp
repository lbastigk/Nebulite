#ifndef NEBULITE_MODULE_BASE_RULESETMODULE_HPP
#define NEBULITE_MODULE_BASE_RULESETMODULE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Base {
/**
 * @class RulesetModule
 * @brief Base class for all ruleset modules, providing functionality to bind static rulesets with metadata and register them into a StaticRulesetMap.
 * @todo Add canonical double pointer GlobalValues struct for all modules, add static polling for values from domain, pass to every ruleset function
 *       Less code duplication, and if we ever decide to allow non-globalspace global contexts, we can easily poll the values from that context
 */
class RulesetModule {
public:
    explicit RulesetModule(std::string_view moduleName);

    /**
     * @brief Registers all static rulesets from this module into the given container
     * @param container The StaticRulesetMap to register into
     */
    void registerModule(Interaction::Rules::StaticRulesetMap& container) const {
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
    static consteval bool isValidTopic(std::string_view const str) {
        return str.starts_with("::") && !str.contains(' ');
    }

    /**
     * @brief helper function to add a static ruleset to this module
     * @tparam topic The topic/name of the ruleset
     * @tparam DerivedRulesetModule The derived RulesetModule type
     * @tparam Func The function implementing the ruleset
     * @param type The type of the ruleset (Local/Global)
     * @param description A brief description of the ruleset's purpose and its used variables
     * @param baseListFunc A function that returns the ordered cache list of base values required by this ruleset, given a context.
     * @todo Add an argument param std::span<std::string> const& args, so that we can have rulesets with arguments such as
     *       ::Controls::PT1 path.to.pt1.object
     *       topic must reduce to the first arg, and we must add the args to the static ruleset object
     */
    template<std::string_view const& topic, typename DerivedRulesetModule, auto Func>
    void bind(
        Interaction::Rules::StaticRuleset::Type type,
        std::string_view description,
        Interaction::Rules::StaticRuleset::BaseListFunction const& baseListFunc
    ){
        assert(Func != nullptr);
        static_assert(isValidTopic(topic), "RulesetModule::bind(): The topic name is not valid. It must start with '::' and contain no spaces.");
        static_assert(std::is_base_of_v<RulesetModule, DerivedRulesetModule>, "RulesetModule::bind(): T must derive from RulesetModule");
        static_assert(std::is_same_v<decltype(DerivedRulesetModule::moduleName), const std::string_view>, "RulesetModule::bind(): DerivedRulesetModule must have a static member 'moduleName' of type std::string_view");
        static_assert(topic.starts_with(DerivedRulesetModule::moduleName), "RulesetModule::bind(): The topic name must start with the module's name as prefix.");
        moduleRulesets.push_back({
            type,
            topic,
            description,
            this,
            [](void* instance, Interaction::Context const& ctx, double** slf, double** otr) {
                auto* self = static_cast<DerivedRulesetModule*>(instance);
                (self->*Func)(ctx, slf, otr);
            },
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
     * @tparam KeyEnum An enumeration type representing the keys for base values.
     */
    template<typename KeyEnum>
    static double& baseVal(double** v, KeyEnum k) noexcept {
        static_assert(std::is_same_v<std::underlying_type_t<KeyEnum>, std::uint8_t>, "baseVal: keyEnum must be an enumeration with underlying type std::uint8_t");
        assert(v != nullptr);
        return *v[static_cast<std::uint8_t>(k)]; // NOLINT
    }

    /**
     * @brief Generates a BaseList-ensurer function for any provided keys.
     * @param baseKeys The key list to retrieve
     * @return The BaseList-ensurer function.
     */
    [[nodiscard]] Interaction::Rules::StaticRuleset::BaseListFunction generateBaseListFunction(std::vector<Data::ScopedKeyView> const& baseKeys) const ;

    /**
     * @brief Checks if the global context is the actual GlobalSpace, and throws an exception if not.
     * @details This is required for function relying on pre-cached global variables
     * @throws std::runtime_error if the global context is not the actual GlobalSpace
     */
    static void checkGlobalContextCorrectness(Interaction::Context const& context);

    /**
     * @brief Checks if the global context is the actual GlobalSpace
     * @details This is required for function relying on pre-cached global variables
     * @return true if the global context is correct, false otherwise
     */
    static bool isGlobalContextCorrect(Interaction::Context const& context);

private:
    // Vector of all static rulesets from this module
    std::vector<Interaction::Rules::StaticRulesetMap::StaticRulesetWithMetadata> moduleRulesets;

    // Unique identifier for caching
    std::uint64_t const id;
};
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_RULESETMODULE_HPP
