#include "Nebulite.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Rules/RulesetModules/Debug.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Debug::message(ContextBase const& context) {
    auto slf = context.self.getDoc().get<uint32_t>(Nebulite::Constants::KeyNames::RenderObject::id, 0);
    Nebulite::cout() << "[DEBUG] Static ruleset '::message' invoked! Context ID: "
                     << std::to_string(slf)
                     << Nebulite::endl;
}

void Debug::error(ContextBase const& context) {
    auto slf = context.self.getDoc().get<uint32_t>(Nebulite::Constants::KeyNames::RenderObject::id, 0);
    Nebulite::cout() << "[DEBUG] Static ruleset '::error' invoked! Context ID: "
                     << std::to_string(slf)
                     << Nebulite::endl;
}

void Debug::whoInteracts(ContextBase const& context) {
    auto slf = context.self.getDoc().get<uint32_t>(Nebulite::Constants::KeyNames::RenderObject::id, 0);
    auto otr = context.other.getDoc().get<uint32_t>(Nebulite::Constants::KeyNames::RenderObject::id, 0);
    std::string message = "[DEBUG] Static ruleset '::whoInteracts' invoked! Self ID: " + std::to_string(slf) + ", Other ID: " + std::to_string(otr) + Nebulite::endl;
    Nebulite::cout() << message;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules
