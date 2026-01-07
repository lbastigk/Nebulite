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
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction {

/**
 * @struct ContextBase
 * @brief Context structure passed to static ruleset functions and other interaction functions.
 *        Contains references to the 'self', 'other', and 'global' domains, with base domain types.
 */
struct ContextBase {
    Execution::DomainBase& self;
    Execution::DomainBase& other;
    Execution::DomainBase& global;
    // TODO: Parent context?
};

/**
 * @struct ContextScope
 * @brief Reduced context structure using JsonScopeBase references.
 *        Useful for functions that only need access to the JSON scopes of the domains.
 */
struct ContextScope {
    Data::JsonScopeBase& self;
    Data::JsonScopeBase& other;
    Data::JsonScopeBase& global;
    // TODO: Parent context?
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
