#include "DomainModule/RenderObject/Debug.hpp"

#include "Core/RenderObject.hpp"

#include "Nebulite.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Update

Constants::Error Debug::update() {
    // For on-tick-updates
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

// NOLINTNEXTLINE
Constants::Error Debug::eval(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    std::string const argStr = Utility::StringHandler::recombineArgs(args);
    Interaction::Context const context{caller, caller, Global::instance()};    // Both self and other are this RenderObject?
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(argStr, context);
    (void)callerScope; // Unused
    return caller.parseStr(argsEvaluated);
}

} // namespace Nebulite::DomainModule::RenderObject
