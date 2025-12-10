#include "Nebulite.hpp"
#include "Interaction/Rules/RulesetModules/Debug.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Debug::message(Context const& context) {
    Nebulite::cout() << "[DEBUG] Static ruleset '::message' invoked!" << Nebulite::endl;
}

void Debug::error(Context const& context) {
    Nebulite::cout() << "[DEBUG] Static ruleset '::error' invoked!" << Nebulite::endl;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules