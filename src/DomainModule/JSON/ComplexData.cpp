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


Constants::Error ComplexData::evaluateMember(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    // Get the member value
    auto const fullKey = callerScope.getRootScope() + args[1];
    if (callerScope.memberType(fullKey) != Data::KeyType::value) {
        // If it's not a value, we can't evaluate it as an expression, so we do nothing
        return Constants::ErrorTable::NONE();
    }

    // Evaluate the string as an expression and set the member to the result
    auto const expressionStr = Interaction::Logic::Expression::removeOuterAntiEvalWrapper(callerScope.get<std::string>(fullKey));
    Interaction::Context const ctx{caller, caller, Global::instance()};
    auto const result = Interaction::Logic::Expression::evalAsJson(expressionStr, ctx);
    callerScope.setSubDoc(fullKey, result);
    return Constants::ErrorTable::NONE();
}

Constants::Error ComplexData::evaluateRecursive(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScopeBase& callerScope){
    std::function<void(Data::ScopedKey const&)> recursiveEvaluate = [&](auto const& key) -> void {
        switch (callerScope.memberType(key)) {
            case Data::KeyType::value:
                {
                    // If it's a value, we try to evaluate it as an expression
                    auto const expressionStr = Interaction::Logic::Expression::removeOuterAntiEvalWrapper(callerScope.get<std::string>(key));
                    Interaction::Context const ctx{caller, caller, Global::instance()};
                    auto const result = Interaction::Logic::Expression::evalAsJson(expressionStr, ctx);
                    callerScope.setSubDoc(key, result);
                    break;
                }
            case Data::KeyType::object:
            case Data::KeyType::array:
                {
                    for (auto const& member : callerScope.listAvailableKeys(key)) {
                        recursiveEvaluate(member);
                    }
                    break;
                }

            case Data::KeyType::null:
            default:
                // If it's not a value, object, or array, we do nothing
                break;
        }
    };

    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    auto const fullKey = callerScope.getRootScope() + args[1];
    recursiveEvaluate(fullKey);
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::JsonScope
