#include "Nebulite.hpp"
#include "Module/Domain/Common/ComplexData.hpp"

namespace Nebulite::Module::Domain::Common {

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

// NOLINTNEXTLINE
Constants::Event ComplexData::jsonSet(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    // Argument parsing
    std::string const& myKey = args[1];
    std::string const expression = Utility::StringHandler::recombineArgs(args.subspan(2));

    // Evaluate
    auto const result = Interaction::Logic::Expression::evalAsJson(expression, ctxScope);
    ctxScope.self.setSubDoc(ctxScope.self.getRootScope() + myKey, result);
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event ComplexData::evaluateMember(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }

    // Get the member value
    auto const fullKey = ctxScope.self.getRootScope() + args[1];
    if (ctxScope.self.memberType(fullKey) != Data::KeyType::value) {
        // If it's not a value, we can't evaluate it as an expression, so we do nothing
        return Constants::Event::Success;
    }

    // Evaluate the string as an expression and set the member to the result
    auto const expressionStr = ctxScope.self.get<std::string>(fullKey).value_or("");
    auto const result = Interaction::Logic::Expression::evalAsJson(expressionStr, ctxScope);
    ctxScope.self.setSubDoc(fullKey, result);
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event ComplexData::evaluateRecursive(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    std::function<void(Data::ScopedKey const&)> recursiveEvaluate = [&](auto const& key) -> void {
        switch (ctxScope.self.memberType(key)) {
            case Data::KeyType::value:
                {
                    // If it's a value, we try to evaluate it as an expression
                    auto const expressionStr = ctxScope.self.get<std::string>(key).value_or("");
                    auto const result = Interaction::Logic::Expression::evalAsJson(expressionStr, ctxScope);
                    ctxScope.self.setSubDoc(key, result);
                    break;
                }
            case Data::KeyType::object:
            case Data::KeyType::array:
                {
                    for (auto const& member : ctxScope.self.listAvailableKeys(key)) {
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

    auto lock = ctxScope.self.lock(); // Lock the domain for thread-safe access
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }

    auto const fullKey = ctxScope.self.getRootScope() + args[1];
    recursiveEvaluate(fullKey);
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Common
