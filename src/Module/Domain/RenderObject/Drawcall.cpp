//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Constants/Event.hpp"
#include "Constants/StandardCapture.hpp"
#include "Core/RenderObject.hpp"
#include "Interaction/Context.hpp"
#include "Module/Domain/RenderObject/Drawcall.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {

//------------------------------------------
// Update

Constants::Event Drawcall::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::Success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Drawcall::drawcallParse(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const drawcallName = args[1];
    auto const drawcallArgs = std::string(__FUNCTION__) + " " + Utility::StringHandler::recombineArgs(args.subspan(2));
    return domain.parseDrawcallCommand(drawcallName, drawcallArgs, ctx, ctxScope);
}

Constants::Event Drawcall::drawcallList(Interaction::Context const& ctx, Interaction::ContextScope&) const {
    for (auto const& drawcallName : domain.listDrawcalls()) {
        ctx.self.capture.log.println(drawcallName);
    }
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::RenderObject
