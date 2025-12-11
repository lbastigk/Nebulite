#include "Nebulite.hpp"
#include "DomainModule/RenderObject/Ruleset.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Interaction/Rules/RulesetCompiler.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Basics

Constants::Error Ruleset::update() {
    //------------------------------------------
    // Verify id is valid (not zero)
    if (id == 0) {
        id = domain->getDoc()->get<uint32_t>(Constants::keyName.renderObject.id,0);
    }

    //------------------------------------------
    // Check all Rulesets
    else if (Nebulite::global().getInvoke() != nullptr) {    // TODO: Nullptr check should not be necessary? Perhaps also change getInvoke to return a reference instead of pointer?
        //------------------------------------------
        // 1.) Reload invokes if needed
        if (reloadRulesets) {
            auto mtx = domain->getDoc()->lock();
            Interaction::Rules::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, domain);
            reloadRulesets = false;
        }

        //------------------------------------------
        // 2.) Directly solve local invokes (loop)
        for (auto const& entry : rulesetsLocal) {
            if (Interaction::Invoke::checkRulesetLogicalCondition(entry->logicalArg, entry->selfPtr)) {
                Nebulite::global().getInvoke()->applyRuleset(entry);
            }
        }

        //------------------------------------------
        // 3.) Checks this object against all conventional invokes
        //	   Manipulation happens at the Invoke::update routine later on
        //     This just generates pairs that need to be updated
        for (size_t idx = 0; idx < subscription_size; idx++) {
            std::string key = Constants::keyName.renderObject.invokeSubscriptions + "[" + std::to_string(idx) + "]";
            auto const subscription = domain->getDoc()->get<std::string>(key, "");
            Nebulite::global().getInvoke()->listen(domain, subscription, id);
        }

        //------------------------------------------
        // 4.) Append general invokes from object itself back for global check
        //     This makes sure that no invokes from inactive objects stay in the list
        for (auto const& entry : rulesetsGlobal) {
            // add pointer to invoke command to global
            Nebulite::global().getInvoke()->broadcast(entry);
        }
    } else {
        return Constants::ErrorTable::RENDERER::CRITICAL_INVOKE_NULLPTR();
    }
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