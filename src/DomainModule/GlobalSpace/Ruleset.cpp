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
    //Interaction::Rules::Construction::RulesetCompiler::parseSingle()
    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

Constants::Error Ruleset::listen(std::span<std::string const> const args) {
    return Constants::ErrorTable::FUNCTIONAL::FEATURE_NOT_IMPLEMENTED();
}

} // namespace Nebulite::DomainModule::GlobalSpace
