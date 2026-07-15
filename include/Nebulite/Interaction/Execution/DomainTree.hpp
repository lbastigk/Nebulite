#ifndef NEBULITE_INTERACTION_EXECUTION_DOMAINTREE_HPP
#define NEBULITE_INTERACTION_EXECUTION_DOMAINTREE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"

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
using DomainTree = Utility::Args::FuncTree<Constants::Event, Context&, ContextScope&>;

} // namespace Nebulite::Interaction::Execution
#endif // NEBULITE_INTERACTION_EXECUTION_DOMAINTREE_HPP
