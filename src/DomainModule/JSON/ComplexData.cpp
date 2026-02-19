#include "Nebulite.hpp"
#include "Core/JsonScope.hpp"
#include "DomainModule/JsonScope/ComplexData.hpp"

namespace Nebulite::DomainModule::JsonScope {

//------------------------------------------
// Update
Constants::Error ComplexData::update() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Error ComplexData::querySet() {
    auto lock = domain.lock(); // Lock the domain for thread-safe access
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}

// NOLINTNEXTLINE
Constants::Error ComplexData::jsonSet(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    // Argument parsing
    std::string const& myKey = args[1];
    std::string const expression = Interaction::Logic::Expression::removeOuterAntiEvalWrapper(args.subspan(2));

    // Evaluate
    Interaction::Context const ctx{caller, caller, Global::instance()};
    auto const result = Interaction::Logic::Expression::evalAsJson(expression, ctx);
    callerScope.setSubDoc(callerScope.getRootScope() + myKey, result);
    return Constants::ErrorTable::NONE();
}
} // namespace Nebulite::DomainModule::JsonScope
