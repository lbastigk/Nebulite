#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"


namespace Nebulite::DomainModule::GlobalSpace {

Constants::Error Ruleset::update() {
    // Currently, no periodic update logic is required for the Ruleset module.
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Constants::Error Ruleset::broadcast(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    if (auto const rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(args[1], domain); rs.has_value()) {
        domain.broadcast(rs.value());
        return Constants::ErrorTable::NONE();
    }
    return Constants::ErrorTable::RULESET::CRITICAL_RULESET_PARSING_FAILED();
}

Constants::Error Ruleset::listen(std::span<std::string const> const& args) const {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto const listener = make_shared<Interaction::Rules::Listener>(domain, args[1]);
    domain.listen(listener);
    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::GlobalSpace
