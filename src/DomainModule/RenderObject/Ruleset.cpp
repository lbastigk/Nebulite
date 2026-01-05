#include "Nebulite.hpp"
#include "DomainModule/RenderObject/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Utility/StringHandler.hpp"

namespace Nebulite::DomainModule::RenderObject {

//------------------------------------------
// Basics

Constants::Error Ruleset::update() {
    //------------------------------------------
    // Verify id is valid (not zero)
    // An id of zero means the RenderObject is outside the Renderer/RenderObjectContainer scope
    // and should not be updated
    if (id == 0) {
        id = domain.getId();
    }

    //------------------------------------------
    // Check all Rulesets
    if (id != 0) {
        // Reload rulesets if needed
        if (reloadRulesets) {
            auto mtx = getDoc().lock();
            Interaction::Rules::Construction::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, domain);
            reloadRulesets = false;
        }

        // Directly apply local rulesets
        for (auto const& entry : rulesetsLocal) {
            if (entry->evaluateCondition()) {
                entry->apply();
            }
        }

        // Listen to broadcasts from subscribed topics
        for (size_t idx = 0; idx < subscription_size; idx++) {
            auto const key = Constants::KeyNames::RenderObject::Ruleset::listen + "[" + std::to_string(idx) + "]";
            auto const subscription = getDoc().get<std::string>(key, "");
            Nebulite::global().listen(domain, subscription, id);
        }

        // Broadcast global rulesets
        for (auto const& entry : rulesetsGlobal) {
            // add pointer to invoke command to global
            Nebulite::global().broadcast(entry);
        }
    } else {
        return Constants::ErrorTable::RENDERER::CRITICAL_INVOKE_NULLPTR();
    }
    return Constants::ErrorTable::NONE();
}

//------------------------------------------
// Available Functions

Constants::Error Ruleset::once(std::span<std::string const> const& args) {
    if (args.size() > 1) {
        std::string const arg = Utility::StringHandler::recombineArgs(args.subspan(1));
        if (auto const rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(arg, domain); rs.has_value()) {
            if (rs.value()->isGlobal()) {
                Nebulite::global().broadcast(rs.value());
            }
            else {
                rs.value()->apply();
            }
            return Constants::ErrorTable::NONE();
        }
        // TODO: Better, custom error for invalid ruleset parsing
        return Constants::ErrorTable::FUNCTIONAL::CRITICAL_INVALID_ARGC_ARGV_PARSING();
    }
    return Constants::ErrorTable::FUNCTIONAL::TOO_FEW_ARGS();
}

Constants::Error Ruleset::reload() {
    reloadRulesets = true;
    return Constants::ErrorTable::NONE();
}

} // namespace Nebulite::DomainModule::RenderObject
