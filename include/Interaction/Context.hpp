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
#include <optional>
#include <string_view>
#include <utility>

//------------------------------------------
namespace Nebulite::Interaction {

class ContextDeriver {
public:
    enum class Type {
        self,
        other,
        global,
        resource
    };

    static Type getTypeFromString(std::string_view const& str);

    static std::pair<Type, std::string_view> getTypeAndPrefixFromString(std::string_view const& str);

private:
    // TODO: Change to "self:", "other:", "global:", same for expression and any tests, documentation and scripts
    //       lots of work, but is more consistent with the naming of read-only-docs: <link>:<key>
    //       So overall it is: <context>:<key.path.traversal>|<transformations>

    static std::string_view constexpr startSelf = "self.";
    static std::string_view constexpr startOther = "other.";
    static std::string_view constexpr startGlobal = "global.";
};

template <typename Target>
class ContextTemplate {
public:
    Target& self;
    Target& other;
    Target& global;

    std::optional<std::reference_wrapper<Target>> getTargetFromType(ContextDeriver::Type const& type) const {
        switch (type) {
            case ContextDeriver::Type::self:
                return self;
            case ContextDeriver::Type::other:
                return other;
            case ContextDeriver::Type::global:
                return global;
            case ContextDeriver::Type::resource:
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
