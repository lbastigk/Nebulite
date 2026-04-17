//------------------------------------------
// Includes

// Nebulite
#include "Nebulite.hpp"
#include "Core/RenderObject.hpp"
#include "Module/Domain/RenderObject/Ruleset.hpp"
#include "Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::DomainModule::RenderObject {

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
                auto const key = Constants::KeyNames::RenderObject::Ruleset::listen + "[" + std::to_string(idx) + "]";
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

Constants::Event Ruleset::once(std::span<std::string const> const& args) const {
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

Constants::Event Ruleset::reload() {
    reloadRulesets = true;
    return Constants::Event::Success;
}

} // namespace Nebulite::DomainModule::RenderObject
