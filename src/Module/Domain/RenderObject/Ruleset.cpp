//------------------------------------------
// Includes

// Nebulite
#include "Core/RenderObject.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Interaction/Rules/Listener.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Module/Domain/RenderObject/Ruleset.hpp"
#include "Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::RenderObject {

//------------------------------------------
// Basics

Constants::Event Ruleset::updateHook() {
    if (initialized) {
        // Reload rulesets if needed
        if (reloadRulesets) {
            auto mtx = moduleScope.lock();
            Interaction::Rules::Construction::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, domain);

            listeners.clear();
            for (size_t idx = 0; idx < subscription_size; idx++) {
                auto const key = Constants::KeyNames::RenderObject::Ruleset::listen.addIndex(idx);
                auto const subscription = moduleScope.get<std::string>(key).value_or("");
                auto listener = std::make_shared<Interaction::Rules::Listener>(domain, subscription);
                listeners.push_back(listener);
            }

            reloadRulesets = false;
        }

        // Directly apply local rulesets
        for (auto const& entry : rulesetsLocal) {
            if (entry->evaluateCondition()) {
                entry->apply();
            }
        }

        // Listen to broadcasts from subscribed topics
        for (auto const& listener : listeners) {
            Global::instance().listen(listener);
        }

        // Broadcast global rulesets
        for (auto const& entry : rulesetsGlobal) {
            // add pointer to invoke command to global
            Global::instance().broadcast(entry);
        }
    }
    else {
        initialized = true;
    }
    return Constants::Event::Success;
}

void Ruleset::reinit() {
    reloadRulesets = true;
    initialized = false;
    subscription_size = moduleScope.memberSize(Constants::KeyNames::RenderObject::Ruleset::listen);
}

//------------------------------------------
// Available Functions

Constants::Event Ruleset::reload() {
    reloadRulesets = true;
    return Constants::Event::Success;
}

} // namespace Nebulite::Module::Domain::RenderObject
