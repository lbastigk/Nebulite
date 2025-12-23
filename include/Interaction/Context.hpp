/**
 * @file Context.hpp
 * @brief Defines context structures for interaction of rulesets and functions
 *        within the Nebulite engine.
 */
#ifndef NEBULITE_INTERACTION_CONTEXT_HPP
#define NEBULITE_INTERACTION_CONTEXT_HPP

//------------------------------------------
// Forward declarations
namespace Nebulite::Core {
class RenderObject;
class GlobalSpace;
} // namespace Nebulite::Core

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
    Interaction::Execution::DomainBase& self;
    Interaction::Execution::DomainBase& other;
    Interaction::Execution::DomainBase& global;
    // TODO: Parent context?
};

/**
 * @brief Struct Context
 * @brief Specialized context structure with concrete RenderObject and GlobalSpace types.
 *        Should only be used if the ContextBase is not sufficient. For now, commented out
 *        to discourage its use.
 */
/*
struct Context {
    Nebulite::Core::RenderObject& self;
    Nebulite::Core::RenderObject& other;
    Nebulite::Core::GlobalSpace& global;
    // TODO: Parent context?
};
*/

} // namespace Nebulite::Interaction
#endif // NEBULITE_INTERACTION_CONTEXT_HPP
