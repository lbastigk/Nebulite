#ifndef INTERACTION_EXECUTION_DOMAINTREE_HPP
#define INTERACTION_EXECUTION_DOMAINTREE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Constants/Event.hpp"
#include "Interaction/Execution/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Interaction::Execution {

/**
 * @brief Using a FuncTree that provides access to the Context of the interaction
 * @details Context& provides access to the domains itself, and ContextScope& to the provided scopes of every interaction member.
 */
using DomainTree = FuncTree<Constants::Event, Context&, ContextScope&>;

} // namespace Nebulite::Interaction::Execution
#endif // INTERACTION_EXECUTION_DOMAINTREE_HPP
