#include "Nebulite.hpp"
#include "Interaction/Rules/RulesetModules/Debug.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Debug::message(ContextBase const& context) const {
    auto const slf = getBaseList(context.self, baseKeys);
    Log::println("[DEBUG] Static ruleset '::message' invoked! Context ID: ", baseVal(slf, Key::id));
}

void Debug::error(ContextBase const& context) const {
    auto const slf = getBaseList(context.self, baseKeys);
    Log::println("[DEBUG] Static ruleset '::error' invoked! Context ID: ", baseVal(slf, Key::id));
}

void Debug::whoInteracts(ContextBase const& context) const {
    auto const slf = getBaseList(context.self, baseKeys);
    auto const otr = getBaseList(context.other, baseKeys);
    Log::println("[DEBUG] Static ruleset '::whoInteracts' invoked! Self ID: ", baseVal(slf, Key::id), ", Other ID: ", baseVal(otr, Key::id));
}

} // namespace Nebulite::Interaction::Rules::RulesetModules
