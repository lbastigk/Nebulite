/**
 * @file Context.hpp
 * @brief Defines context structures for interaction of rulesets and functions
 *        within the Nebulite engine.
 */
#ifndef NEBULITE_INTERACTION_CONTEXT_HPP
#define NEBULITE_INTERACTION_CONTEXT_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <optional>
#include <string_view>
#include <utility>

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Interaction {

class ContextDeriver {
public:
    enum class TargetType {
        self,
        other,
        global,
        resource,
        local,
        full,
        none
    };

    /**
     * @brief used to strip any context prefix from a key
     * @details Removes the beginning, if applicable.
     *          Does not remove the beginning context for resource variables,
     *          as the beginning is needed for the link.
     * @param str The string to strip the context from.
     * @return The string without its context prefix.
     */
    static std::string stripContext(std::string_view const& str);

    /**
     * @brief used to strip any context prefix from a key
     * @details Removes the beginning, if applicable.
     *          Does not remove the beginning context for resource variables,
     *          as the beginning is needed for the link.
     * @param str The string to strip the context from, is modified!
     */
    static void stripContextFromView(std::string_view& str);

    /**
     * @brief Gets the context from a key before it's stripped
     * @details If the string doesn't start with a known prefix, it is considered a resource variable.
     * @param str The string to get the context from.
     * @return The string of the key.
     */
    static TargetType getTypeFromString(std::string_view const& str);

    /**
     * @brief Gets the context and type from a key before it's stripped
     * @details If the string doesn't start with a known prefix, it is considered a resource variable. The returned prefix string is the full given string.
     * @param str The string to get the context and prefix from
     * @return The Target and string_view as pair.
     */
    static std::pair<TargetType, std::string_view> getTypeAndPrefixFromString(std::string_view const& str);

    static auto constexpr contextKeySeparator = ':';
private:
    /**
     * @brief Pairs of context types and their corresponding prefixes for easy reference when parsing variable keys.
     * @details The character contextKeySeparator after the corresponding prefix is required.
     */
    static std::array<std::pair<TargetType, std::string_view>, 5> constexpr contextPrefixPairs = {
        std::make_pair(TargetType::self, "self"),
        std::make_pair(TargetType::other, "other"),
        std::make_pair(TargetType::local, "local"),
        std::make_pair(TargetType::global, "global"),
        std::make_pair(TargetType::full, "full")
    };
};

template <typename Target>
class ContextTemplate {
public:
    Target& self;
    Target& other;
    Target& global; // The global target, must be the owner of self and other to outlive them! Otherwise, some context storages may break!

    [[nodiscard]] std::optional<std::reference_wrapper<Target>> getTargetFromType(ContextDeriver::TargetType const& type) const {
        switch (type) {
            case ContextDeriver::TargetType::self:
                return self;
            case ContextDeriver::TargetType::other:
                return other;
            case ContextDeriver::TargetType::global:
                return global;
            // Unsupported
            case ContextDeriver::TargetType::resource:
            case ContextDeriver::TargetType::local:
            case ContextDeriver::TargetType::full:
            case ContextDeriver::TargetType::none:
                return std::nullopt;
            default:
                std::unreachable();
        }
    }
};

// ContextScope -> JSON scope access with JsonScope references
//                 Access to the scoped data only
// Context      -> Full domain access

/**
 * @struct ContextScope
 * @brief Reduced context structure using JsonScope references.
 *        Useful for functions that only need access to the JSON scopes of the domains.
 * @details Third layer of abstraction, only JSON scope access available.
 */
class ContextScope : public ContextTemplate<Data::JsonScope> {
public:
    /**
     * @brief Combines all scopes into a single JSON scope, with keys prefixed by their context (self., other., global.).
     * @param merged The scope to populate
     */
    void combineAll(Data::JsonScope& merged) const ;

    /**
     * @brief Combines only the local scopes (self and other) into a single JSON scope, with keys prefixed by their context (self., other.).
     * @param merged The scope to populate
     */
    void combineLocal(Data::JsonScope& merged) const ;
};

/**
 * @struct Context
 * @brief Context structure passed to static ruleset functions and other interaction functions.
 *        Contains references to the 'self', 'other', and 'global' domains, with base domain types.
 * @details Second layer of abstraction, only base domain functionality available.
 */
class Context : public ContextTemplate<Execution::Domain>{};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
