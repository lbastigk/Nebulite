//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Rules/Listener.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules {

Listener::Listener(Execution::Domain& d, std::string const& t) : domain(d), topic(t) {
    if (auto const& entry = StaticRulesetMap::getInstance().getStaticRulesetByName(t); entry.type != StaticRulesetMap::StaticRuleSetWithMetaData::invalid) {
        // Static ruleset, ensure list of required double values
        otr = entry.baseListFunc(domain);
    }
    // JSON ruleset or unknown static ruleset, no list required
}

} // namespace Nebulite::Interaction::Rules
