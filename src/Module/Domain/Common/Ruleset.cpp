//------------------------------------------
// Includes

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Interaction/Rules/Listener.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Module/Domain/Common/Ruleset.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

Constants::Event Ruleset::updateHook() {
    // Currently, no periodic update logic is required for the Ruleset module.
    return Constants::Event::Success;
}

//------------------------------------------
// Available Functions

Constants::Event Ruleset::invokeOnce(std::span<std::string const> const& args) const {
    if (args.size() > 1) {
        std::string const arg = Utility::StringHandler::recombineArgs(args.subspan(1));
        if (auto const rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(arg, domain); rs.has_value()) {
            if (rs.value()->isGlobal()) {
                Global::instance().broadcast(rs.value());
            }
            else {
                rs.value()->apply();
            }
            return Constants::Event::Success;
        }
        domain.capture.warning.println("Failed to parse ruleset: " + arg);
        return Constants::Event::Warning;
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
}

} // namespace Nebulite::Module::Domain::GlobalSpace
