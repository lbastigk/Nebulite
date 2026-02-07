#include "Nebulite.hpp"
#include "RulesetModule/Debug.hpp"

namespace Nebulite::RulesetModule {

void Debug::message(Interaction::Context const& context) const {
    auto const slf = getBaseList(context.self, baseKeys);
    Log::println("[DEBUG] Static ruleset '::message' invoked! Context ID: ", baseVal(slf, Key::id));
}

void Debug::error(Interaction::Context const& context) const {
    auto const slf = getBaseList(context.self, baseKeys);
    Log::println("[DEBUG] Static ruleset '::error' invoked! Context ID: ", baseVal(slf, Key::id));
}

void Debug::whoInteracts(Interaction::Context const& context) const {
    auto const slf = getBaseList(context.self, baseKeys);
    auto const otr = getBaseList(context.other, baseKeys);
    Log::println("[DEBUG] Static ruleset '::whoInteracts' invoked! Self ID: ", baseVal(slf, Key::id), ", Other ID: ", baseVal(otr, Key::id));
}

} // namespace Nebulite::RulesetModule
