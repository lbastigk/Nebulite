//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Graphics/DearImGui/Core.hpp"
#include "Nebulite/Graphics/DearImGui/DomainViewer.hpp"
#include "Nebulite/Interaction/Logic/Expression.hpp"
#include "Nebulite/Module/Domain/Common/General.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Promise.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

Constants::Event General::updateHook() {
    if (imguiViewEnabled && Graphics::DearImGui::Core::checkImguiReadyForRendering()) {
        if (!lastContext.valid()) {
            // Context is not valid, disable imgui view and log error
            imguiViewEnabled = false;
            domain.capture.error.println("Failed to render ImGui view: Context is no longer valid. Disabling ImGui view.");
            return Constants::Event::error;
        }
        Global::instance().getRenderer().addRenderCallback([&] {
            auto ctx = Interaction::Context{
                {
                    .self = *lastContext.self,
                    .other = *lastContext.self,
                    .global = *lastContext.global,
                },
            };
            auto ctxScope = Interaction::ContextScope{
                {
                    .self = *lastContext.selfScope,
                    .other = *lastContext.selfScope,
                    .global = *lastContext.globalScope,
                },
            };
            Graphics::DearImGui::DomainViewer::render(ctx, ctxScope, domain.capture, domain.getName());
        });
    }
    return Constants::Event::success;
}

// [BASIC]

Constants::Event General::echo(Utility::Args::SegmentedStringView const& args) const {
    domain.capture.log.println(args.recombineSubspan(1));
    return Constants::Event::success;
}

Constants::Event General::ifFunc(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
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
    auto const condition = args.subspan(1, conditionEnd);
    auto const commands = args.subspan(commandStart);

    // condition must start with $( and end with ): Simple expressions are boolean-convertible
    if (!condition.beginsWith("$(") || !condition.endsWith(')')) {
        return Constants::StandardCapture::Warning::Functional::unknownArg(ctx.self.capture);
    }

    // Conditional check
    // We ensured that the condition is wrapped in $() above, but there could still be parsing errors (missing parenthesis etc.)
    Interaction::Logic::Expression const expr(condition);
    if (!expr.isReturnableAsBool()) {
        ctx.self.capture.error.println("Critical Error: A custom if-condition failed.\nCondition failed: " + condition.recombine() + " is not a boolean expression.");
        return Constants::Event::warning;
    }
    if (expr.evalAsBool(ctxScope, Utility::Promise<Utility::PromiseType::FunctionVerified, &Interaction::Logic::Expression::isReturnableAsBool>{})) {
        std::vector<std::string_view> newCommands{__FUNCTION__};
        commands.copySubspan(newCommands, 0);
        return ctx.self.parse(newCommands, ctx, ctxScope);
    }
    return Constants::Event::success;
}

Constants::Event General::forFunc(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() > 4) {
        auto const varName = '{' + std::string(args[1]) + '}';
        int const idxStart = std::stoi(Interaction::Logic::Expression::eval(args[2], ctxScope));
        int const idxEnd = std::stoi(Interaction::Logic::Expression::eval(args[3], ctxScope));

        std::vector newArgs{std::string(args[0])};
        args.copySubspan(newArgs, 4);
        auto containsVar = newArgs
            | std::views::transform([varName](std::string const& arg) {
                return arg.contains(varName);
            })
            | std::ranges::to<std::vector<bool>>();

        // Rare edge case: if the caller arg, arg[0], contains the varName, do not overwrite!
        // Should never happen, but just in case. Replacing it could make debugging harder.
        containsVar[0] = false;

        for (int i = idxStart; i <= idxEnd; i++) {
            auto const replacer = std::to_string(i);
            for (auto const [index, arg] : newArgs | Utility::Ranges::enumerate) {
                if (containsVar[index]) {
                    auto const originalIndex = 3 + index;
                    arg = Utility::StringHandler::replaceAll(args[originalIndex], varName, replacer);
                }
            }
            if (auto const event = ctx.self.parse(newArgs, ctx, ctxScope); event != Constants::Event::success) {
                return event;
            }
        }
        return Constants::Event::success;
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
}

Constants::Event General::forFuncProgress(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() > 4) {
        std::size_t constexpr barWidth = 50;

        auto const varName = '{' + std::string(args[1]) + '}';
        int const idxStart = std::stoi(Interaction::Logic::Expression::eval(args[2], ctxScope));
        int const idxEnd = std::stoi(Interaction::Logic::Expression::eval(args[3], ctxScope));

        std::vector newArgs{std::string(args[0])};
        args.copySubspan(newArgs, 4);
        auto containsVar = newArgs
            | std::views::transform([varName](std::string const& arg) {
                return arg.contains(varName);
            })
            | std::ranges::to<std::vector<bool>>();

        // Rare edge case: if the caller arg, arg[0], contains the varName, do not overwrite!
        // Should never happen, but just in case. Replacing it could make debugging harder.
        containsVar[0] = false;

        for (int i = idxStart; i <= idxEnd; i++) {
            // Provide progress bar only to cout for now
            double const progress = static_cast<double>(i - idxStart) / static_cast<double>(idxEnd - idxStart + 1);
            std::cout << "[";
            auto const pos = static_cast<size_t>(barWidth * progress);
            for (std::size_t barIdx = 0; barIdx < barWidth; ++barIdx) {
                if (barIdx < pos) std::cout << "=";
                else if (barIdx == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << static_cast<int>(progress * 100.0) << " %\r";
            std::cout.flush();

            // Replace
            auto const replacer = std::to_string(i);
            for (auto const [index, arg] : newArgs | Utility::Ranges::enumerate) {
                if (containsVar[index]) {
                    auto const originalIndex = 3 + index;
                    arg = Utility::StringHandler::replaceAll(args[originalIndex], varName, replacer);
                }
            }

            // Parse
            if (auto const event = ctx.self.parse(newArgs, ctx, ctxScope); event != Constants::Event::success) {
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

        return Constants::Event::success;
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
}

Constants::Event General::assertFunc(Utility::Args::SegmentedStringView const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const condition = args.subspan(1);

    // condition must start with $( and end with ): Simple expressions are boolean-convertible
    if (!condition.beginsWith("$(") || !condition.endsWith(')')) {
        return Constants::StandardCapture::Warning::Functional::unknownArg(ctx.self.capture);
    }

    // Conditional check
    // We ensured that the condition is wrapped in $() above, but there could still be parsing errors (missing parenthesis etc.)
    Interaction::Logic::Expression const expr(condition);
    if (!expr.isReturnableAsBool()) {
        ctx.self.capture.error.println("Critical Error: A custom assertion failed.\nAssertion failed: " + condition.recombine() + " is not a boolean expression.");
        return Constants::Event::error;
    }
    if (!expr.evalAsBool(ctxScope, Utility::Promise<Utility::PromiseType::FunctionVerified, &Interaction::Logic::Expression::isReturnableAsBool>{})) {
        ctx.self.capture.error.println("Critical Error: A custom assertion failed.\nAssertion failed: " + condition.recombine() + " is not true.");
        return Constants::Event::error;
    }

    // All good
    return Constants::Event::success;
}

Constants::Event General::nop() {
    // Do nothing
    return Constants::Event::success;
}

// [FORWARD/REPARSE]

Constants::Event General::forwardToOther(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    return ctx.other.parse(args, ctx, ctxScope);
}

Constants::Event General::forwardToGlobal(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    return ctx.global.parse(args, ctx, ctxScope);
}

Constants::Event General::reparseInOther(Utility::Args::SegmentedStringView const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    Interaction::Context otherCtx{
        {
            .self = ctx.other,
            .other = ctx.self,
            .global = ctx.global,
        },
    };
    Interaction::ContextScope otherCtxScope{
        {
            .self = ctxScope.other,
            .other = ctxScope.self,
            .global = ctxScope.global,
        },
    };
    return ctx.other.parse(args, otherCtx, otherCtxScope);
}

Constants::Event General::reparseInGlobal(Utility::Args::SegmentedStringView const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    Interaction::Context globalCtx{
        {
            .self = ctx.global,
            .other = ctx.global,
            .global = ctx.global,
        },
    };
    Interaction::ContextScope globalCtxScope{
        {
            .self = ctxScope.global,
            .other = ctxScope.global,
            .global = ctxScope.global,
        },
    };
    return ctx.global.parse(args, globalCtx, globalCtxScope);
}

// [IMGUI]

Constants::Event General::imguiView(Utility::Args::SegmentedStringView const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope) {
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
    return Constants::Event::success;
}

// [OTHER]

Constants::Event General::capture(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }

    // Parse
    auto const argsToParse = args.subspan(1); // Kind of breaks contract that args[0] is the caller, as it'll be the key...
    auto result = Constants::Event::success;
    auto history = ctx.self.capture.redirectHistory([&] {
        result = ctx.self.parse(argsToParse, ctx, ctxScope);
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

Constants::Event General::eval(Utility::Args::SegmentedStringView const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope){
    // TODO: An idea would be to only eval until the next "eval" keyword, allowing for nested evals within for-loops, ifs, etc.:
    //       Example:
    //       eval for i 1 {global.loopCount} eval process-state {global.currentState} {i}
    //       This way, the first eval will not vanish the information within the for-loop,
    //       allowing us to properly retrieve the current state for each iteration.
    //       Do the same for the RenderObject eval function. Perhaps we should combine them?

    Interaction::Logic::Expression const expr(args.subspan(0));

    // Evaluate expression, empty context for self and other
    auto const argsEvaluated = expr.eval(ctxScope);

    // reparse
    return ctx.self.parseStr(argsEvaluated, ctx, ctxScope);
}


} // namespace Nebulite::Module::Domain::Common
