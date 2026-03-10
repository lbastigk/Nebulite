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

namespace Nebulite::Interaction {

// ContextScopeBase -> JSON scope access with JsonScope references
//                     Access to the scoped data only
// Context          -> Full domain access

/**
 * @struct ContextScopeBase
 * @brief Reduced context structure using JsonScope references.
 *        Useful for functions that only need access to the JSON scopes of the domains.
 * @details Third layer of abstraction, only JSON scope access available.
 */
class ContextScopeBase {
public:
    Data::JsonScope& self;
    Data::JsonScope& other;
    Data::JsonScope& global;
    // TODO: Parent context?
};

/**
 * @struct Context
 * @brief Context structure passed to static ruleset functions and other interaction functions.
 *        Contains references to the 'self', 'other', and 'global' domains, with base domain types.
 * @details Second layer of abstraction, only base domain functionality available.
 */
class Context {
public:
    Execution::Domain& self;
    Execution::Domain& other;
    Execution::Domain& global;
    // TODO: Parent context?

    friend class Logic::Expression; // For context demotion
private:
    [[nodiscard]] ContextScopeBase demote() const ;
};

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
