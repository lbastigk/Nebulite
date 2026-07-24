//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Graphics/ImguiHelper.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Domain/Common/General.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

// [BASICS]

Constants::Event General::updateHook() {
    if (imguiViewEnabled && Graphics::ImguiHelper::checkImguiReadyForRendering()) {
        if (!lastContext.valid()) {
            // Context is not valid, disable imgui view and log error
            imguiViewEnabled = false;
            domain.capture.error.println("Failed to render ImGui view: Context is no longer valid. Disabling ImGui view.");
            return Constants::Event::Error;
        }
        Global::instance().getRenderer().addRenderCallback([&] {
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
        });
    }
    return Constants::Event::Success;
}

Constants::Event General::capture(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    // Parse
    auto const argsToParse = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto result = Constants::Event::Success;
    auto history = ctx.self.capture.redirectHistory([&] {
        result = ctx.self.parseStr(argsToParse, ctx, ctxScope);
    });

    // Set
    auto const key = ctxScope.self.getRootScope().addMember(args[1]);
    ctxScope.self.setEmptyArray(key);
    for (auto const [i, line] : history | Utility::Ranges::enumerate) {
        auto indexedKey = key.addIndex(i);
        ctxScope.self.set<std::string>(indexedKey, line.content);
    }
    return result;
}

Constants::Event General::eval(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
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

Constants::Event General::echo(std::span<std::string_view const> const& args) const {
    domain.capture.log.println(Utility::StringHandler::recombineArgs(args.subspan(1)));
    return Constants::Event::Success;
}

Constants::Event General::func_if(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    // See if any arg is "then", if so, only evaluate until then, and execute the rest as commands if the condition is true
    auto commandStartFinder = [&]-> std::pair<size_t, std::size_t> {
        if (auto const it = std::ranges::find(args, std::string("then")); it != args.end()) {
            auto const commandStart = static_cast<size_t>(std::distance(args.begin(), it));

            // remove whitespaces
            for (std::size_t conditionEnd = commandStart-1; conditionEnd > 0; conditionEnd--) {
                if (!args[conditionEnd].empty() && args[conditionEnd] != " ") {
                    return {conditionEnd, commandStart+1};
                }
            }
            std::unreachable(); // trailing whitespace args should be impossible, we must find one that isn't a whitespace before then
        }
        return {1,2};
    };

    auto const [conditionEnd, commandStart] = commandStartFinder();
    std::string const condition = Utility::StringHandler::recombineArgs(args.subspan(1, conditionEnd));
    std::string commands = Utility::StringHandler::recombineArgs(args.subspan(commandStart));

    // condition must start with $( and end with ): Simple expressions are boolean-convertible
    if (!condition.starts_with("$(") || !condition.ends_with(")")) {
        return Constants::StandardCapture::Warning::Functional::unknownArg(ctx.self.capture);
    }

    // Conditional check
    // We ensured that the condition is wrapped in $() above, but there could still be parsing errors (missing parenthesis etc.)
    Interaction::Logic::Expression const expr(condition);
    if (!expr.getEvaluationInfo().simpleExpression) {
        ctx.self.capture.error.println("Critical Error: A custom if-condition failed.\nCondition failed: " + condition + " is not a boolean expression.");
        return Constants::Event::Warning;
    }
    if (expr.evalAsBool(ctxScope)) {
        commands = __FUNCTION__ + std::string(" ") + commands;
        return ctx.self.parseStr(commands, ctx, ctxScope);
    }
    return Constants::Event::Success;
}

Constants::Event General::func_assert(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    std::string const& condition = Utility::StringHandler::recombineArgs(args.subspan(1));

    // condition must start with $( and end with ): Simple expressions are boolean-convertible
    if (condition.front() != '$' || condition[1] != '(' || condition.back() != ')') {
        return Constants::StandardCapture::Warning::Functional::unknownArg(ctx.self.capture);
    }

    // Conditional check
    // We ensured that the condition is wrapped in $() above, but there could still be parsing errors (missing parenthesis etc.)
    Interaction::Logic::Expression const expr(condition);
    if (!expr.getEvaluationInfo().simpleExpression) {
        ctx.self.capture.error.println("Critical Error: A custom assertion failed.\nAssertion failed: " + condition + " is not a boolean expression.");
        return Constants::Event::Error;
    }
    if (!expr.evalAsBool(ctxScope)) {
        ctx.self.capture.error.println("Critical Error: A custom assertion failed.\nAssertion failed: " + condition + " is not true.");
        return Constants::Event::Error;
    }

    // All good
    return Constants::Event::Success;
}

Constants::Event General::func_for(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() > 4) {
        auto const& varName = std::string(args[1]);

        int const iStart = std::stoi(Interaction::Logic::Expression::eval(args[2], ctxScope));
        int const iEnd = std::stoi(Interaction::Logic::Expression::eval(args[3], ctxScope));

        std::vector argsVec = {std::string(args[0])};
        for (int i = iStart; i <= iEnd; i++) {
            argsVec.resize(1);
            auto const replacer = std::to_string(i);
            for (auto const& arg : args.subspan(4)) {
                argsVec.push_back(Utility::StringHandler::replaceAll(arg, '{' + varName + '}', replacer));
            }
            if (auto const event = ctx.self.parse(argsVec, ctx, ctxScope); event != Constants::Event::Success) {
                return event;
            }
        }
        return Constants::Event::Success;
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
}

Constants::Event General::func_forProgress(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() > 4) {
        std::size_t constexpr barWidth = 50;

        auto const& varName = std::string(args[1]);

        int const iStart = std::stoi(Interaction::Logic::Expression::eval(args[2], ctxScope));
        int const iEnd = std::stoi(Interaction::Logic::Expression::eval(args[3], ctxScope));

        std::vector argsVec = {std::string(args[0])};
        for (int i = iStart; i <= iEnd; i++) {
            // Provide progress bar only to cout for now
            double const progress = static_cast<double>(i - iStart) / static_cast<double>(iEnd - iStart + 1);
            std::cout << "[";
            auto const pos = static_cast<size_t>(barWidth * progress);
            for (std::size_t barIdx = 0; barIdx < barWidth; ++barIdx) {
                if (barIdx < pos) std::cout << "=";
                else if (barIdx == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << static_cast<int>(progress * 100.0) << " %\r";
            std::cout.flush();

            // Parse
            argsVec.resize(1);
            auto const replacer = std::to_string(i);
            for (auto const& arg : args.subspan(4)) {
                argsVec.push_back(Utility::StringHandler::replaceAll(arg, '{' + varName + '}', replacer));
            }
            if (auto const event = ctx.self.parse(argsVec, ctx, ctxScope); event != Constants::Event::Success) {
                return event;
            }
        }
        // Final progress bar
        std::cout << "[";
        for (std::size_t barIdx = 0; barIdx < barWidth; ++barIdx) {
            std::cout << "=";
        }
        std::cout << "] " << 100 << " %\r";
        std::cout.flush();
        std::cout << "\n";

        return Constants::Event::Success;
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
}

Constants::Event General::nop(std::span<std::string_view const> const& /*args*/) {
    // Do nothing
    return Constants::Event::Success;
}

// [FORWARD/REPARSE]

Constants::Event General::forwardToOther(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args);
    return ctx.other.parseStr(argStr, ctx, ctxScope);
}

Constants::Event General::forwardToGlobal(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args);
    return ctx.global.parseStr(argStr, ctx, ctxScope);
}

Constants::Event General::reparseInOther(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args);
    Interaction::Context otherCtx{
        {
            .self = ctx.other,
            .other = ctx.self,
            .global = ctx.global
        }
    };
    Interaction::ContextScope otherCtxScope{
        {
            .self = ctxScope.other,
            .other = ctxScope.self,
            .global = ctxScope.global
        }
    };
    return ctx.other.parseStr(argStr, otherCtx, otherCtxScope);
}

Constants::Event General::reparseInGlobal(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    std::string const argStr = Utility::StringHandler::recombineArgs(args);

    Interaction::Context globalCtx{
        {
            .self = ctx.global,
            .other = ctx.global,
            .global = ctx.global
        }
    };
    Interaction::ContextScope globalCtxScope{
        {
            .self = ctxScope.global,
            .other = ctxScope.global,
            .global = ctxScope.global
        }
    };

    return ctx.global.parseStr(argStr, globalCtx, globalCtxScope);
}

// [IMGUI]

Constants::Event General::imguiView(std::span<std::string_view const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(ctx.self.capture);
    }
    if (args[1] == "on") {
        // We store the context of 'self' and 'global', as their lifetime should exceed that of the updateHook
        // This isn't textbook, as we make a huge assumption about the architecture.
        // But so far, context global is always the same and outlives the context self at all times.
        // Later on we may need a better interaction system, where each element using a global context is automatically deleted if the global context is deleted
        // But that isn't necessary at the moment.
        lastContext.self = &ctx.self;
        lastContext.global = &ctx.global;
        lastContext.selfScope = &ctxScope.self;
        lastContext.globalScope = &ctxScope.global; // We actually need the global context so the ImGui console calls can propagate it to different functions!
        imguiViewEnabled = true;
    } else if (args[1] == "off") {
        imguiViewEnabled = false;
    } else {
        return Constants::StandardCapture::Warning::Functional::unknownArg(domain.capture);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::Common
