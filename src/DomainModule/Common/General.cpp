#include "DomainModule/Common/General.hpp"
#include "Graphics/ImguiHelper.hpp"
#include "Nebulite.hpp"

namespace Nebulite::DomainModule::Common {

Constants::Error General::update() {
    if (imguiViewEnabled) {
        Graphics::ImguiHelper::renderDomain(domain, domain.capture, *lastImguiCallerScope, domain.getName());
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::imguiView(std::span<std::string const> const& args, Interaction::Execution::Domain& /*caller*/, Data::JsonScope& callerScope) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    lastImguiCallerScope = &callerScope; // Store caller scope for use in imgui view
    if (args[1] == "on") {
        imguiViewEnabled = true;
    } else if (args[1] == "off") {
        imguiViewEnabled = false;
    } else {
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error General::eval(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope){
    // TODO: An idea would be to only eval until the next "eval" keyword, allowing for nested evals within for-loops, ifs, etc.:
    //       Example:
    //       eval for i 1 {global.loopCount} eval process-state {global.currentState} {i}
    //       This way, the first eval will not vanish the information within the for-loop,
    //       allowing us to properly retrieve the current state for each iteration.
    //       Do the same for the RenderObject eval function. Perhaps we should combine them?

    // argc/argv to string for evaluation
    std::string const argStr = Utility::StringHandler::recombineArgs(args);

    // Evaluate expression, empty context for self and other
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(argStr);

    // reparse
    (void)callerScope; // Unused parameter
    return caller.parseStr(argsEvaluated);
}

Constants::Error General::echo(std::span<std::string const> const& args) const {
    domain.capture.log.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return Constants::ErrorTable::NONE();
}

// NOLINTNEXTLINE
Constants::Error General::func_if(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    if (args.size() < 3) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (!Interaction::Logic::Expression::evalAsBool(args[1])) {
        // If the condition is false/nan, skip the following commands
        return Constants::ErrorTable::NONE();
    }

    // Build the command string from rest
    std::string commands = Utility::StringHandler::recombineArgs(args.subspan(2));
    commands = __FUNCTION__ + std::string(" ") + commands;
    (void)callerScope; // Unused parameter
    return caller.parseStr(commands);
}

Constants::Error General::func_assert(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }

    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    std::string const& condition = args[1];

    // condition must start with $( and end with )
    if (condition.front() != '$' || condition[1] != '(' || condition.back() != ')') {
        return Constants::ErrorTable::FUNCTIONAL::UNKNOWN_ARG();
    }

    // Evaluate condition
    if (!Interaction::Logic::Expression::evalAsBool(condition)) {
        return Constants::ErrorTable::addError("Critical Error: A custom assertion failed.\nAssertion failed: " + condition + " is not true.", Constants::Error::CRITICAL);
    }

    // All good
    return Constants::ErrorTable::NONE();
}

Constants::Error General::func_return(std::span<std::string const> const& args) {
    return Constants::ErrorTable::addError(Utility::StringHandler::recombineArgs(args.subspan(1)), Constants::Error::CRITICAL);
}

// NOLINTNEXTLINE
Constants::Error General::func_for(std::span<std::string const> const& args, Interaction::Execution::Domain& caller, Data::JsonScope& callerScope) {
    if (args.size() > 4) {
        std::string const& varName = args[1];

        int const iStart = std::stoi(Interaction::Logic::Expression::eval(args[2]));
        int const iEnd = std::stoi(Interaction::Logic::Expression::eval(args[3]));

        std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(4));
        for (int i = iStart; i <= iEnd; i++) {
            // for + args
            std::string args_replaced = std::string(args[0]) + " " + Utility::StringHandler::replaceAll(argStr, '{' + varName + '}', std::to_string(i));
            (void)callerScope; // Unused parameter
            if (auto const err = caller.parseStr(args_replaced); err.isCritical()) {
                return err;
            } else if (err.isError()) {
                caller.capture.error.println(err.getDescription());
            }
        }
    }
    return Constants::ErrorTable::NONE();
}

Constants::Error General::nop(std::span<std::string const> const& /*args*/) {
    // Do nothing
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::Common
