#pragma once

#include "Constants/StandardCapture.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/FuncTree.hpp"

namespace Nebulite::Interaction::Execution {

/**
 * @brief Using a FuncTree that provides access to the Context of the interaction
 * @details Context& provides access to the domains itself, and ContextScope& to the provided scopes of every interaction member.
 */
using DomainTree = FuncTree<Constants::Event, Context&, ContextScope&>;

} // namespace Nebulite::Interaction::Execution
