#include "Nebulite.hpp"
#include "Module/Domain/Common/ComplexData.hpp"

namespace Nebulite::DomainModule::Common {

//------------------------------------------
// Update
Constants::Event ComplexData::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

// NOLINTNEXTLINE
Constants::Event ComplexData::querySet() {
    return Constants::StandardCapture::Error::Functional::functionNotImplemented(domain.capture);
}

Constants::Event ComplexData::jsonSet(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(caller.capture);
    }

    // Argument parsing
    std::string const& myKey = args[1];
    std::string const expression = Utility::StringHandler::recombineArgs(args.subspan(2));

    // Evaluate
    Interaction::Context const ctx{caller, caller, Global::instance()};
    auto const result = Interaction::Logic::Expression::evalAsJson(expression, ctx);
    callerScope.setSubDoc(callerScope.getRootScope() + myKey, result);
    return Constants::Event::Success;
}


Constants::Event ComplexData::evaluateMember(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    auto lock = callerScope.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(caller.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(caller.capture);
    }

    // Get the member value
    auto const fullKey = callerScope.getRootScope() + args[1];
    if (callerScope.memberType(fullKey) != Data::KeyType::value) {
        // If it's not a value, we can't evaluate it as an expression, so we do nothing
        return Constants::Event::Success;
    }

    // Evaluate the string as an expression and set the member to the result
    auto const expressionStr = callerScope.get<std::string>(fullKey).value_or("");
    Interaction::Context const ctx{caller, caller, Global::instance()};
    auto const result = Interaction::Logic::Expression::evalAsJson(expressionStr, ctx);
    callerScope.setSubDoc(fullKey, result);
    return Constants::Event::Success;
}

Constants::Event ComplexData::evaluateRecursive(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope){
    std::function<void(Data::ScopedKey const&)> recursiveEvaluate = [&](auto const& key) -> void {
        switch (callerScope.memberType(key)) {
            case Data::KeyType::value:
                {
                    // If it's a value, we try to evaluate it as an expression
                    auto const expressionStr = callerScope.get<std::string>(key).value_or("");
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
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(caller.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(caller.capture);
    }

    auto const fullKey = callerScope.getRootScope() + args[1];
    recursiveEvaluate(fullKey);
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::Common
