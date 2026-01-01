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

Constants::Error Ruleset::broadcast(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    auto rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(args[1], domain);
    if (rs.has_value()) {
        domain->broadcast(rs.value());
        return Constants::ErrorTable::NONE();
    }
    return Constants::ErrorTable::RULESET::CRITICAL_RULESET_PARSING_FAILED();
}

Constants::Error Ruleset::listen(std::span<std::string const> const& args) {
    if (args.size() < 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
    }
    if (args.size() > 2) {
        return Constants::ErrorTable::FUNCTIONAL::TOO_MANY_ARGS();
    }
    // TODO: Is Listener ID zero properly reserved for GlobalSpace?
    //       create a proper static object id handler with an enum for reserved ids?
    domain->listen(domain, args[1], 0);
    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::GlobalSpace
