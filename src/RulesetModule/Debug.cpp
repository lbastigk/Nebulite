#include "Nebulite.hpp"
#include "RulesetModule/Debug.hpp"

namespace Nebulite::RulesetModule {

void Debug::message(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    Global::capture().log.println("[DEBUG] Static ruleset '::message' invoked! Domain ID: ", context.self.getId());
}

void Debug::error(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    Global::capture().log.println("[DEBUG] Static ruleset '::error' invoked! Domain ID: ", context.self.getId());
}

void Debug::whoInteracts(Interaction::Context const& context, double**& slf, double**& otr) const {
    ensureBaseList(context.self, baseKeys, slf);
    ensureBaseList(context.other, baseKeys, otr);
    Global::capture().log.println("[DEBUG] Static ruleset '::whoInteracts' invoked! Domain self ID: ", context.self.getId(), ", Domain other ID: ", context.other.getId());
}

} // namespace Nebulite::RulesetModule
