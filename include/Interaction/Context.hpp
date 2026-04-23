/**
 * @file Context.hpp
 * @brief Defines context structures for interaction of rulesets and functions
 *        within the Nebulite engine.
 */
#ifndef NEBULITE_INTERACTION_CONTEXT_HPP
#define NEBULITE_INTERACTION_CONTEXT_HPP

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction::Logic {
class Expression; // For Context demotion to ContextScope
} // namespace Nebulite::Interaction::Logic

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <optional>
#include <string_view>
#include <utility>

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
    static std::string_view stripContext(std::string_view const& str);

    /**
     * @brief Gets the context from a key before it's stripped
     * @details If the string doesn't start with a known prefix, it is considered a resource variable.
     * @param str The string to get the context from.
     * @return The string of the key.
     */
    static TargetType getTypeFromString(std::string_view const& str);

    static std::pair<TargetType, std::string_view> getTypeAndPrefixFromString(std::string_view const& str);

private:
    // TODO: Change to "self:", "other:", "global:", same for expression and any tests, documentation and scripts
    //       lots of work, but is more consistent with the naming of read-only-docs: <link>:<key>
    //       So overall it is: <context>:<key.path.traversal>|<transformations>

    /**
     * @brief Pairs of context types and their corresponding prefixes for easy reference when parsing variable keys.
     */
    static std::array<std::pair<TargetType, std::string_view>, 5> constexpr contextPrefixPairs = {
        std::make_pair(TargetType::self, "self."),
        std::make_pair(TargetType::other, "other."),
        std::make_pair(TargetType::local, "local."),
        std::make_pair(TargetType::global, "global."),
        std::make_pair(TargetType::full, "full.")
    };
};

template <typename Target>
class ContextTemplate {
public:
    Target& self;
    Target& other;
    Target& global;

    std::optional<std::reference_wrapper<Target>> getTargetFromType(ContextDeriver::TargetType const& type) const {
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
class ContextScope : public ContextTemplate<Data::JsonScope> {};

/**
 * @struct Context
 * @brief Context structure passed to static ruleset functions and other interaction functions.
 *        Contains references to the 'self', 'other', and 'global' domains, with base domain types.
 * @details Second layer of abstraction, only base domain functionality available.
 */
class Context : public ContextTemplate<Execution::Domain>{
public:
    // Context Demotion access
    // Expression requires demotion so we deduce the full scope of any domain for evaluation.
    // We assume that any written expression acts on the root scope of the domain, unless specified differently
    friend class Logic::Expression;
private:
    /**
     * @brief Demotes the full Domain to its inner data. Since the JSON data of a Domain is private,
     *        demotion is only permitted for few selected classes that require full access.
     * @return The scope of each context member
     */
    [[nodiscard]] ContextScope demote() const ;
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
