#ifndef NEBULITE_MODULE_BASE_RULESETMODULE_HPP
#define NEBULITE_MODULE_BASE_RULESETMODULE_HPP

//------------------------------------------
// Includes

// Standard library
#include <cassert>
#include <concepts>
#include <cstdint> // NOLINT
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/GlobalValue.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite/Module/Base/RulesetModuleConcept.hpp"
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
 */
class RulesetModule {
    // Vector of all static rulesets from this module
    std::vector<Interaction::Rules::StaticRulesetMap::StaticRulesetWithMetadata> moduleRulesets;

    // Unique identifier for caching
    std::uint64_t const id;
public:
    /**
     * @brief Constructs a RulesetModule with a unique identifier based on the module name.
     * @tparam DerivedModule The derived module type, which must satisfy the DerivedFromRulesetModule concept.
     *                       Used to ensure that any module deriving from RulesetModule adheres to the expected interface and behavior.
     * @param moduleName The name of the module, used to generate a unique identifier.
     * @param prefix The optional prefix for global access using getRulesetModuleAccessToken.
     *               This way, the privilege of a module can be restricted to a subscope of the entire GlobalSpace document.
     *               Any enhancement is a deliberate choice by modifying the prefix.
     */
    template<DerivedFromRulesetModule DerivedModule>
    explicit RulesetModule(std::string_view const moduleName, DerivedModule* /*derived*/, std::optional<std::string> const& prefix = std::nullopt)
        : id{Data::MappedOrderedCacheList::generateUniqueId(moduleName)}
        , globalAccessScopePrefix{prefix} {}

    /**
     * @brief Registers all static rulesets from this module into the given container
     * @param container The StaticRulesetMap to register into
     */
    void registerModule(Interaction::Rules::StaticRulesetMap& container) const {
        for (auto const& ruleset : moduleRulesets) {
            container.bindStaticRuleset(ruleset);
        }
    }

    // Optional prefix for global access scope, if provided
    std::optional<std::string> const globalAccessScopePrefix;

protected:
    /**
     * @brief Helper function to get a RulesetModuleToken for a derived module
     * @param derivedModule The derived RulesetModule instance, used for the prefix
     * @return A RulesetModuleToken for the derived module
     * @throw std::runtime_error if the derived module does not have a global access scope prefix
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
     * @tparam Topic The topic/name of the ruleset
     * @tparam DerivedRulesetModule The derived RulesetModule type
     * @param type The type of the ruleset (Local/Global)
     * @param func The function implementing the ruleset
     * @param description A brief description of the ruleset's purpose and its used variables
     * @param baseListFunc A function that returns the ordered cache list of base values required by this ruleset, given a context.
     * @todo Add an argument param std::span<std::string> const& args, so that we can have rulesets with arguments such as
     *       ::Controls::PT1 path.to.pt1.object
     *       topic must reduce to the first arg, and we must add the args to the static ruleset object
     */
    template<std::string_view const& Topic, typename DerivedRulesetModule>
    void bindMemberFunction(
        void (DerivedRulesetModule::*func)(Interaction::Context const&, double**, double**, Interaction::GlobalValueCopy const&) const,
        Interaction::Rules::StaticRuleset::BaseListFunction const& baseListFunc,
        Interaction::Rules::StaticRuleset::Type const& type,
        std::string_view const description
    ){
        assert(func != nullptr);
        static_assert(isValidTopic(Topic), "RulesetModule::bindMemberFunction(): The topic name is not valid. It must start with '::' and contain no spaces.");
        static_assert(Topic.starts_with(DerivedRulesetModule::moduleName), "RulesetModule::bindMemberFunction(): The topic name must start with the module's name as prefix.");
        static_assert(std::is_base_of_v<RulesetModule, DerivedRulesetModule>, "RulesetModule::bindMemberFunction(): T must derive from RulesetModule");
        static_assert(std::is_same_v<decltype(DerivedRulesetModule::moduleName), std::string_view const>, "RulesetModule::bindMemberFunction(): DerivedRulesetModule must have a static member 'moduleName' of type std::string_view");
        moduleRulesets.push_back({
            type,
            Topic,
            description,
            [this, func](Interaction::Context const& ctx, double** slf, double** otr, Interaction::GlobalValueCopy const& global){
                (static_cast<DerivedRulesetModule const*>(this)->*func)(ctx, slf, otr, global);
            },
            baseListFunc,
        });
    }

    template<std::string_view const& Topic, typename DerivedRulesetModule>
    void bindStaticFunction(
        void (*func)(Interaction::Context const&, double**, double**, Interaction::GlobalValueCopy const&),
        Interaction::Rules::StaticRuleset::BaseListFunction const& baseListFunc,
        Interaction::Rules::StaticRuleset::Type const& type,
        std::string_view const description
    ){
        assert(func != nullptr);
        static_assert(isValidTopic(Topic), "RulesetModule::bindStaticFunction(): The topic name is not valid. It must start with '::' and contain no spaces.");
        static_assert(Topic.starts_with(DerivedRulesetModule::moduleName), "RulesetModule::bindStaticFunction(): The topic name must start with the module's name as prefix.");
        static_assert(std::is_base_of_v<RulesetModule, DerivedRulesetModule>, "RulesetModule::bindStaticFunction(): T must derive from RulesetModule");
        static_assert(std::is_same_v<decltype(DerivedRulesetModule::moduleName), std::string_view const>, "RulesetModule::bindStaticFunction(): DerivedRulesetModule must have a static member 'moduleName' of type std::string_view");
        moduleRulesets.push_back({
            type,
            Topic,
            description,
            [func](Interaction::Context const& ctx,double** slf,double** otr,Interaction::GlobalValueCopy const& global) {
                func(ctx, slf, otr, global);
            },
            baseListFunc,
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
    template <std::ranges::input_range R> requires std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<R>>,Data::ScopedKeyView>
    [[nodiscard]] Interaction::Rules::StaticRuleset::BaseListFunction generateBaseListFunction(R&& baseKeys) const {
        return [this, bk = std::forward<R>(baseKeys)](Interaction::Execution::Domain const& domain) -> double** {
            try {
                return domain.ensureOrderedCacheList(id, bk);
            } catch (...) {
                return nullptr;
            }
        };
    }

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
};
} // namespace Nebulite::Module::Base
#endif // NEBULITE_MODULE_BASE_RULESETMODULE_HPP
