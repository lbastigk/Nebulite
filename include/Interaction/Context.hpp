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
class JsonScopeBase;
} // namespace Nebulite::Data

namespace Nebulite::Interaction::Execution {
class DomainBase;
class Domain;
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction {

// ContextScopeBase -> JSON scope access with JsonScopeBase references
//                     Access to the scoped data only
// ContextBase      -> Base domain access with DomainBase references
//                     Additional Access to parsing, functree inheriting, update routines.
// ContextFull      -> Full domain access with specific domain types
//                     Additional Access to class of the domain,
//                     module initialization and updating.

/**
 * @struct ContextScopeBase
 * @brief Reduced context structure using JsonScopeBase references.
 *        Useful for functions that only need access to the JSON scopes of the domains.
 * @details Third layer of abstraction, only JSON scope access available.
 */
class ContextScopeBase {
public:
    Data::JsonScopeBase& self;
    Data::JsonScopeBase& other;
    Data::JsonScopeBase& global;
    // TODO: Parent context?
};

/**
 * @struct ContextBase
 * @brief Context structure passed to static ruleset functions and other interaction functions.
 *        Contains references to the 'self', 'other', and 'global' domains, with base domain types.
 * @details Second layer of abstraction, only base domain functionality available.
 */
class ContextBase {
public:
    Execution::DomainBase& self;
    Execution::DomainBase& other;
    Execution::DomainBase& global;
    // TODO: Parent context?
private:
    [[nodiscard]] ContextScopeBase demote() const ;
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
