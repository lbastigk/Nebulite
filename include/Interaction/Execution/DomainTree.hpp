#pragma once

#include "Constants/StandardCapture.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/FuncTree.hpp"

namespace Nebulite::Interaction::Execution {

using DomainTree = FuncTree<Constants::Event, Context&, ContextScope&>;

} // namespace Nebulite::Interaction::Execution
