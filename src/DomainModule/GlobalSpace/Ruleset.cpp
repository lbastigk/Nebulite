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

Constants::Error Ruleset::broadcast(std::span<std::string const> const args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    auto rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(args[1], domain);
    if (rs.has_value()) {
        domain->getInvoke().broadcast(rs.value());
        return Constants::ErrorTable::NONE();
    }
    return Constants::ErrorTable::RULESET::CRITICAL_RULESET_PARSING_FAILED();
}

Constants::Error Ruleset::listen(std::span<std::string const> const args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }

    domain->getInvoke().listen(domain, args[1], 0); // TODO: Listener ID zero reserved for GlobalSpace?

    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::GlobalSpace
