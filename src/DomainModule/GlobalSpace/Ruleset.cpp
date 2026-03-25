#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"


namespace Nebulite::DomainModule::GlobalSpace {

Constants::Event Ruleset::updateHook() {
    // Currently, no periodic update logic is required for the Ruleset module.
    return Constants::Event::Success;
}

//------------------------------------------
// Available Functions

Constants::Event Ruleset::broadcast(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    if (auto const rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(args[1], domain); rs.has_value()) {
        domain.broadcast(rs.value());
        return Constants::Event::Success;
    }
    return Constants::StandardCapture::Error::Ruleset::parsingFailed(domain.capture);
}

Constants::Event Ruleset::listen(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    auto const listener = make_shared<Interaction::Rules::Listener>(domain, args[1]);
    domain.listen(listener);
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::GlobalSpace
