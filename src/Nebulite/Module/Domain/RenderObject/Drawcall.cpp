//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/RenderObject.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Module/Domain/RenderObject/Drawcall.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {

//------------------------------------------
// Update

Constants::Event Drawcall::updateHook() {
    // Add Domain-specific updates here!
    // General rule:
    // This is used to update all variables/states that are INTERNAL ONLY
    return Constants::Event::success;
}

//------------------------------------------
// Domain-Bound Functions

Constants::Event Drawcall::drawcallParse(std::span<std::string_view const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope) const {
    if (args.size() < 3) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(ctx.self.capture);
    }
    auto const& name = args[1];
    auto const drawcallArgs = std::string(__FUNCTION__) + " " + Utility::StringHandler::recombineArgs(args.subspan(2));
    return domain.parseDrawcallCommand(name, drawcallArgs, ctx, ctxScope);
}

Constants::Event Drawcall::drawcallList(Interaction::Context const& ctx, Interaction::ContextScope& /*ctxScope*/) const {
    for (auto const& name : domain.listDrawcalls()) {
        ctx.self.capture.log.println(name);
    }
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::RenderObject
