#include "Graphics/ImguiHelper.hpp"
#include "Module/Domain/Common/General.hpp"
#include "Nebulite.hpp"


namespace Nebulite::Module::Domain::Common {

Constants::Event General::updateHook() {
    if (imguiViewEnabled && Graphics::ImguiHelper::checkImguiReadyForRendering()) {
        if (!lastContext.valid()) {
            // Context is not valid, disable imgui view and log error
            imguiViewEnabled = false;
            domain.capture.error.println("Failed to render ImGui view: Context is no longer valid. Disabling ImGui view.");
            return Constants::Event::Error;
        }
        auto ctx = Interaction::Context{
            {
                .self = *lastContext.self,
                .other = *lastContext.self,
                .global = *lastContext.global
            }
        };
        auto ctxScope = Interaction::ContextScope{
            {
                .self = *lastContext.selfScope,
                .other = *lastContext.selfScope,
                .global = *lastContext.globalScope
            }
        };
        Graphics::ImguiHelper::renderDomain(ctx, ctxScope, domain.capture, domain.getName());
    }
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event General::imguiView(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }

    // We store the context of 'self' and 'global', as their lifetime should exceed that of the updateHook
    // This isn't textbook, as we make a huge assumption about the architecture.
    // But so far, context global is always the same and outlives the context self at all times.
    // Later on we may need a better interaction system, where each element using a global context is automatically deleted if the global context is deleted
    // But that isn't necessary at the moment.
    lastContext.self = &ctx.self;
    lastContext.global = &ctx.global;
    lastContext.selfScope = &ctxScope.self;
    lastContext.globalScope = &ctxScope.global; // We actually need the global context so the ImGui console calls can propagate it to different functions!
    if (args[1] == "on") {
        imguiViewEnabled = true;
    } else if (args[1] == "off") {
        imguiViewEnabled = false;
    } else {
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event General::eval(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    // TODO: An idea would be to only eval until the next "eval" keyword, allowing for nested evals within for-loops, ifs, etc.:
    //       Example:
    //       eval for i 1 {global.loopCount} eval process-state {global.currentState} {i}
    //       This way, the first eval will not vanish the information within the for-loop,
    //       allowing us to properly retrieve the current state for each iteration.
    //       Do the same for the RenderObject eval function. Perhaps we should combine them?

    // argc/argv to string for evaluation
    std::string const argStr = Utility::StringHandler::recombineArgs(args);

    // Evaluate expression, empty context for self and other
    std::string const argsEvaluated = Interaction::Logic::Expression::eval(argStr, ctxScope);

    // reparse
    return ctx.self.parseStr(argsEvaluated, ctx, ctxScope);
}

Constants::Event General::echo(std::span<std::string const> const& args) const {
    domain.capture.log.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event General::func_if(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    // See if any arg is "then", if so, only evaluate until then, and execute the rest as commands if the condition is true
    auto commandStartFinder = [&] {
        size_t start = 2;
        if (auto const it = std::ranges::find(args, std::string("then")); it != args.end()) {
            auto const idx = std::distance(args.begin(), it);
            start = static_cast<size_t>(idx) + 1;
        }
        return start;
    };

    // Conditional check
    if (size_t const commandStart = commandStartFinder(); Interaction::Logic::Expression::evalAsBool(Utility::StringHandler::recombineArgs(args.subspan(1, commandStart - 1)), ctxScope)) {
        std::string commands = Utility::StringHandler::recombineArgs(args.subspan(commandStart));
        commands = __FUNCTION__ + std::string(" ") + commands;
        return ctx.self.parseStr(commands, ctx, ctxScope);
    }
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event General::func_assert(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }

    std::string const& condition = args[1];

    // condition must start with $( and end with )
    if (condition.front() != '$' || condition[1] != '(' || condition.back() != ')') {
        return Constants::StandardCapture::Warning::Functional::unknownArg(ctx.self.capture);
    }

    // Evaluate condition
    if (!Interaction::Logic::Expression::evalAsBool(condition, ctxScope)) {
        ctx.self.capture.error.println("Critical Error: A custom assertion failed.\nAssertion failed: " + condition + " is not true.");
        return Constants::Event::Error;
    }

    // All good
    return Constants::Event::Success;
}

// NOLINTNEXTLINE
Constants::Event General::func_for(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() > 4) {
        std::string const& varName = args[1];

        int const iStart = std::stoi(Interaction::Logic::Expression::eval(args[2], ctxScope));
        int const iEnd = std::stoi(Interaction::Logic::Expression::eval(args[3], ctxScope));

        std::string const argStr = Utility::StringHandler::recombineArgs(args.subspan(4));
        for (int i = iStart; i <= iEnd; i++) {
            // for + args
            std::string args_replaced = std::string(args[0]) + " " + Utility::StringHandler::replaceAll(argStr, '{' + varName + '}', std::to_string(i));
            if (auto const event = ctx.self.parseStr(args_replaced, ctx, ctxScope); event != Constants::Event::Success) {
                return event;
            }
        }
    }
    return Constants::Event::Success;
}

Constants::Event General::nop(std::span<std::string const> const& /*args*/) {
    // Do nothing
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Common
