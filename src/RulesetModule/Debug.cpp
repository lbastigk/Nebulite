#include "Nebulite.hpp"
#include "RulesetModule/Debug.hpp"

namespace Nebulite::RulesetModule {

void Debug::message(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    Log::println("[DEBUG] Static ruleset '::message' invoked! Context ID: ", baseVal(slf, Key::id));
}

void Debug::error(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    Log::println("[DEBUG] Static ruleset '::error' invoked! Context ID: ", baseVal(slf, Key::id));
}

void Debug::whoInteracts(Interaction::Context const& context, double**& slf, double**& otr) const {
    ensureBaseList(context.self, baseKeys, slf);
    ensureBaseList(context.other, baseKeys, otr);
    Log::println("[DEBUG] Static ruleset '::whoInteracts' invoked! Self ID: ", baseVal(slf, Key::id), ", Other ID: ", baseVal(otr, Key::id));
}

} // namespace Nebulite::RulesetModule
