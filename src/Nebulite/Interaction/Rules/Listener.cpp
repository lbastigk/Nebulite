//------------------------------------------
// Includes

// Standard library
#include <string>
#include <string_view>

// Nebulite
#include "Nebulite/Interaction/Rules/Listener.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Interaction/Rules/StaticRulesetMap.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules {

Listener::Listener(Execution::Domain& d, std::string_view const t) : domain(d), topic(t) {
    if (auto const& entry = StaticRulesetMap::getInstance().getStaticRulesetByName(t); entry.type != StaticRuleset::Type::invalid) {
        // Static ruleset, ensure list of required double values
        otr = entry.baseListFunc(domain);
    }
    // JSON ruleset or unknown static ruleset, no list required
}

} // namespace Nebulite::Interaction::Rules
