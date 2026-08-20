//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cstddef>
#include <cstdint> // NOLINT
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Constants/StandardCapture.hpp"
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Interaction/Rules/Construction/RulesetCompiler.hpp"
#include "Nebulite/Interaction/Rules/Listener.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Module/Domain/Common/Ruleset.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Domain::Common {

Constants::Event Ruleset::updateHook() {
    std::scoped_lock const lock(initializeMutex);
    if (initialized) {
        // Reload rulesets if needed
        if (reloadRulesets) {
            reloadRulesets = false;
            Key const scopedKey(moduleScope);
            auto mtx = moduleScope.lock();
            Interaction::Rules::Construction::RulesetCompiler::parse(rulesetsGlobal, rulesetsLocal, domain, moduleScope.shareScope(scopedKey.broadcast));
            listeners.clear();
            for (std::size_t idx = 0; idx < subscriptionSize; idx++) {
                auto const key = scopedKey.listen.addIndex(idx);
                auto const subscription = moduleScope.get<std::string>(key).value_or("");
                auto listener = std::make_shared<Interaction::Rules::Listener>(domain, subscription);
                listeners.push_back(listener);
            }

            //------------------------------------------
            // Estimate cost of parsed rulesets

            // Local entries
            auto const costLocal = std::ranges::fold_left(
                rulesetsLocal,
                std::uint32_t{0},
                [](std::uint32_t const acc, std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
                    return acc + entry->getEstimatedCost();
                }
            );

            // Global entries
            auto const costGlobal = std::ranges::fold_left(
                rulesetsGlobal,
                std::uint32_t{0},
                [](std::uint32_t const acc, std::shared_ptr<Interaction::Rules::Ruleset> const& entry) {
                    return acc + entry->getEstimatedCost();
                }
            );

            moduleScope.set(scopedKey.costLocal, costLocal);
            moduleScope.set(scopedKey.costGlobal, costGlobal);
            moduleScope.set(scopedKey.costTotal, costLocal + costGlobal);
            noRulesets = rulesetsLocal.empty() && rulesetsGlobal.empty();
        }

        // Early exit if no rulesets are present
        if (noRulesets) {
            return Constants::Event::success;
        }

        // Apply rulesets, using the global space as manager
        auto& globalSpace = Global::instance();

        // Directly apply local rulesets
        for (auto const& entry : rulesetsLocal) {
            if (entry->evaluateConditionLocally(globalSpace)) {
                globalSpace.applyRuleset(*entry);
            }
        }

        // Listen to broadcasts from subscribed topics
        for (auto const& listener : listeners) {
            globalSpace.listen(listener);
        }

        // Broadcast global rulesets
        for (auto const& entry : rulesetsGlobal) {
            // add pointer to invoke command to global
            globalSpace.broadcast(entry);
        }
    }
    else {
        initialized = true;
    }
    return Constants::Event::success;
}

void Ruleset::reinit() {
    std::scoped_lock const lock(initializeMutex);
    Key const scopedKey(moduleScope);
    reloadRulesets = true;
    initialized = false;
    subscriptionSize = moduleScope.memberSize(scopedKey.listen);
}

//------------------------------------------
// Available Functions

Constants::Event Ruleset::reload() {
    reloadRulesets = true;
    return Constants::Event::success;
}

Constants::Event Ruleset::invokeOnce(std::span<std::string_view const> const args) const {
    if (args.size() > 1) {
        std::string const arg = Utility::StringHandler::recombineArgs(args.subspan(1));
        if (auto const rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(arg, domain); rs.has_value()) {
            if (rs.value()->isGlobal()) {
                Global::instance().broadcast(rs.value());
            }
            else {
                Global::instance().applyRuleset(*rs.value());
            }
            return Constants::Event::success;
        }
        domain.capture.warning.println("Failed to parse ruleset: " + arg);
        return Constants::Event::warning;
    }
    return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
}

Constants::Event Ruleset::broadcast(std::span<std::string_view const> const args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }

    if (auto const rs = Interaction::Rules::Construction::RulesetCompiler::parseSingle(args[1], domain); rs.has_value()) {
        Global::instance().broadcast(rs.value());
        return Constants::Event::success;
    }
    return Constants::StandardCapture::Error::Ruleset::parsingFailed(domain.capture);
}

Constants::Event Ruleset::listen(std::span<std::string_view const> const args) const {
    if (args.size() < 2) {
        return Constants::StandardCapture::Warning::Functional::tooFewArgs(domain.capture);
    }
    if (args.size() > 2) {
        return Constants::StandardCapture::Warning::Functional::tooManyArgs(domain.capture);
    }
    auto const listener = make_shared<Interaction::Rules::Listener>(domain, args[1]);
    Global::instance().listen(listener);
    return Constants::Event::success;
}

} // namespace Nebulite::Module::Domain::Common
