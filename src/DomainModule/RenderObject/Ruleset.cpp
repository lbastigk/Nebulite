#include "DomainModule/RenderObject/Ruleset.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Basics

Constants::Error Ruleset::update() {
    // No periodic update needed for now
    // Perhaps later on we wish to verify ruleset integrity or similar tasks
    return Constants::ErrorTable::NONE();
}

std::string const Ruleset::Ruleset_name = "ruleset";
std::string const Ruleset::Ruleset_desc = "Ruleset management functions for the RenderObject domain.";

//------------------------------------------
// Available Functions

Constants::Error Ruleset::once(std::span<std::string const> const& args) {
    // For this to work, we need to leverage the RulesetCompiler in a clever way
    // Use parts of it to turn a string into a ruleset and execute it once
    // If its global, we need to store it somewhere temporarily
    // If its local, we can just execute it directly and discard it

    // auto ruleset = RuleSetCompiler::compileFromString(args[0]);
    // if (ruleset.type == RulesetType::Global) { tmp.appendAndBroadcast(ruleset); }
    // else if (ruleset.type == RulesetType::Local) { executeLocalRuleset(ruleset); }
    return Constants::ErrorTable::FUNCTIONAL::CRITICAL_FUNCTION_NOT_IMPLEMENTED();
}
std::string const Ruleset::once_name = "once";
std::string const Ruleset::once_desc = "Executes a ruleset once from a given string input.\nEither a static ruleset name or a link to a json-defined ruleset.";


} // namespace Nebulite::DomainModule::RenderObject