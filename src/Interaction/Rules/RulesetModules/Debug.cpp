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

void Debug::who(Context const& context) {
    uint32_t slf = context.self.getDoc()->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id, 0);
    uint32_t otr = context.other.getDoc()->get<uint32_t>(Nebulite::Constants::keyName.renderObject.id, 0);
    std::string message = "[DEBUG] Static ruleset '::who' invoked! Self ID: " + std::to_string(slf) + ", Other ID: " + std::to_string(otr) + Nebulite::endl;
    Nebulite::cout() << message;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules