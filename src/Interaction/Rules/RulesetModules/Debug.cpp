#include "Nebulite.hpp"
#include "Constants/KeyNames.hpp"
#include "Interaction/Rules/RulesetModules/Debug.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Debug::message(ContextBase const& context) {
    auto const slf = getBaseList(context.self, keys);
    Nebulite::cout() << "[DEBUG] Static ruleset '::message' invoked! Context ID: "
                     << std::to_string(baseVal(slf, Key::id))
                     << Nebulite::endl;
}

void Debug::error(ContextBase const& context) {
    auto const slf = getBaseList(context.self, keys);
    Nebulite::cout() << "[DEBUG] Static ruleset '::error' invoked! Context ID: "
                     << std::to_string(baseVal(slf, Key::id))
                     << Nebulite::endl;
}

void Debug::whoInteracts(ContextBase const& context) {
    auto const slf = getBaseList(context.self, keys);
    auto const otr = getBaseList(context.other, keys);
    std::string const message = "[DEBUG] Static ruleset '::whoInteracts' invoked! Self ID: " + std::to_string(baseVal(slf, Key::id)) + ", Other ID: " + std::to_string(baseVal(otr, Key::id)) + Nebulite::endl;
    Nebulite::cout() << message;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules
