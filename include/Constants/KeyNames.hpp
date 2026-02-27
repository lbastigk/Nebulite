/**
 * @file KeyNames.hpp
 * @brief Defines constant key names used throughout the Nebulite framework.
 * @details While for DomainModules we can write scoped keys directly within their classes,
 *          this is not feasible for Domain-Related keys due to their hierarchical nature.
 *          Specifically, DomainModules are never dependent on other DomainModules besides their key names.
 *          But Domains can own other Domains, requiring knowledge of their key names.
 *          Therefore, we centralize these key names here to avoid circular dependencies
 *          and to ensure consistency across the framework.
 */

#ifndef NEBULITE_CONSTANTS_KEYNAMES_HPP
#define NEBULITE_CONSTANTS_KEYNAMES_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/ScopedKey.hpp"

//------------------------------------------
// Macros


// Macro to help declare the scope of the struct
#define DECLARE_SCOPE(scopeStr) static auto constexpr scope = scopeStr;

// Macro to declare the scope empty
#define DECLARE_NO_SCOPE() static auto constexpr scope = std::nullopt;

// Helper function to validate scope and create ScopedKeyView
// Performs a compile-time check to ensure the scope exists and has the correct type
template<auto& ScopeRef, typename T>
consteval auto makeScopedHelper(T const& keyStr) {
    static_assert(
        std::is_same_v<std::decay_t<decltype(ScopeRef)>, const char*> ||
        std::is_same_v<std::decay_t<decltype(ScopeRef)>, std::nullopt_t>,
        "MAKE_SCOPED requires DECLARE_SCOPE or DECLARE_NO_SCOPE to be called first in this struct");
    return Nebulite::Data::ScopedKeyView::create<ScopeRef>(keyStr);
}

// Macro to help create a scoped key with the previously declared scope
// Requires a call to DECLARE_SCOPE or DECLARE_NO_SCOPE beforehand to declare the scope member
// This macro performs a compile-time check to ensure 'scope' exists and has the correct type
#define MAKE_SCOPED(keyStr) makeScopedHelper<scope>(keyStr)

/**
 * @class KeyGroup
 * @brief A template class to group related keys under a common scope.
 * @details This class allows for defining a group of related keys that share a common scope.
 *          The scope is defined as an optional template parameter.
 *          If the parameter is not provided, the scope is arbitrary and the keys will be non-scoped.
 *          This is useful for organizing keys related to specific domains or modules within the Nebulite framework.
 * @tparam Prefix An optional fixed string that defines the scope prefix for the keys in this group. If not provided, the keys will be non-scoped.
 */
template <FixedOptionalString Prefix = FixedOptionalString<0>()>
class KeyGroup {
public:
    static consteval auto makeScoped(const char* keyStr) {
        return Nebulite::Data::ScopedKeyView::createFromFixedOptionalString<Prefix>(keyStr);
    }

    static constexpr auto getScope() {
        if constexpr (Prefix.has_scope) {
            return std::string_view(Prefix.value, [] (const char* p) constexpr {
                std::size_t i = 0;
                while (p[i] != '\0') ++i;
                return i;
            }(Prefix.value));
        } else {
            return std::nullopt;
        }
    }
};

//------------------------------------------
namespace Nebulite::Constants {

/**
 * @struct KeyNames
 * @brief Centralized struct for constant key names used throughout the Nebulite framework.
 */
struct KeyNames {
    /**
     * @brief Basic keys related to any domain.
     * @details No scope! As Domains may be inside other Domains, the scope is arbitrary.
     */
    struct Domain : KeyGroup<> {
        static auto constexpr id = makeScoped("id"); // TODO: Use this instead of RenderObject id
    };

    /**
     * @struct Renderer
     * @brief Keys related to the Renderer domain
     * @details The scope is set to "renderer.", meaning the entire renderer lives inside this scope of the GlobalSpace.
     */
    struct Renderer : KeyGroup<"renderer."> {
        static auto constexpr dispResX = makeScoped("resolution.X");
        static auto constexpr dispResY = makeScoped("resolution.Y");
        static auto constexpr dispResXLogical = makeScoped("resolution.logical.X");
        static auto constexpr dispResYLogical = makeScoped("resolution.logical.Y");
        static auto constexpr positionX = makeScoped("position.X");
        static auto constexpr positionY = makeScoped("position.Y");
        static auto constexpr windowScale = makeScoped("resolution.scalar");
    };

    /**
     * @brief Keys in the global space.
     * @details The scope is the root scope.
     */
    struct GlobalSpace : KeyGroup<""> {
        // No keys for now
    };

    /**
     * @brief Keys related to RenderObjects, which are the main entities in the Renderer domain.
     * @details The scope is the root scope, as RenderObjects own their own scope.
     * @todo Consider making the scope arbitrary, as RenderObjects may live inside other scopes
     *       e.g. In GlobalSpace for drafts, or in RenderObjects for child objects
     */
    struct RenderObject : KeyGroup<""> {
        static auto constexpr id = makeScoped("id"); // TODO: Make this part of Domain itself!
        static auto constexpr positionX = makeScoped("posX");
        static auto constexpr positionY = makeScoped("posY");
        static auto constexpr layer = makeScoped("layer");

        static auto constexpr draw = makeScoped("draw");
        static auto constexpr sizeX = makeScoped("size.x");
        static auto constexpr sizeY = makeScoped("size.y");
        static auto constexpr sizeR = makeScoped("size.r");

        // Keys for Ruleset invocations and subscriptions
        // TODO flatten into RenderObject once RenderObject has no scope, turn keys into "ruleset.list" and "ruleset.listen"
        struct Ruleset : KeyGroup<"ruleset."> {
            static auto constexpr list = makeScoped("list");
            static auto constexpr listen = makeScoped("listen");
        };
    };

    /**
     * @brief Keys related to Rulesets, which define the behavior of RenderObjects and other entities.
     * @details No scope, as they are extracted from larger JSON objects and have arbitrary scopes depending on where they are defined.
     *          E.g. `ruleset.list[0]` for the first one, `ruleset.list[1]` for the second one, etc.
     */
    struct Ruleset : KeyGroup<> {
        static auto constexpr topic = makeScoped("topic");
        static auto constexpr condition = makeScoped("condition");
        static auto constexpr assignments = makeScoped("action.assign");
        static auto constexpr parseOnGlobal = makeScoped("action.functioncall.global");
        static auto constexpr parseOnSelf   = makeScoped("action.functioncall.self");
        static auto constexpr parseOnOther  = makeScoped("action.functioncall.other");
    };
};

} // namespace Nebulite::Constants

#endif // NEBULITE_CONSTANTS_KEYNAMES_HPP
