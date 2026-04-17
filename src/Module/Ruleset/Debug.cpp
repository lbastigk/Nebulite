#include "Nebulite.hpp"
#include "Module/Ruleset/Debug.hpp"

namespace Nebulite::RulesetModule {

// NOLINTNEXTLINE
void Debug::message(Interaction::Context const& context, double**& /*slf*/, double**& /*otr*/) const {
    Global::capture().log.println("[DEBUG] Static ruleset '::message' invoked! Domain ID: ", context.self.getId());
}

// NOLINTNEXTLINE
void Debug::error(Interaction::Context const& context, double**& /*slf*/, double**& /*otr*/) const {
    Global::capture().log.println("[DEBUG] Static ruleset '::error' invoked! Domain ID: ", context.self.getId());
}

// NOLINTNEXTLINE
void Debug::whoInteracts(Interaction::Context const& context, double**& /*slf*/, double**& /*otr*/) const {
    Global::capture().log.println("[DEBUG] Static ruleset '::whoInteracts' invoked! Domain self ID: ", context.self.getId(), ", Domain other ID: ", context.other.getId());
}

} // namespace Nebulite::RulesetModule
