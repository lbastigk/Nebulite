//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/GlobalValue.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"
#include "Nebulite/Module/Ruleset/Debug.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/Utility/Io/Capture.hpp"

//------------------------------------------
namespace Nebulite::Module::Ruleset {

Debug::Debug() : RulesetModule(moduleName, this) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Local
    bindStaticFunction<messageName, Debug>(&Debug::message, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, messageDesc);
    bindStaticFunction<errorName, Debug>(&Debug::error, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, errorDesc);

    // Global
    bindStaticFunction<whoInteractsName, Debug>(&Debug::whoInteracts, baseListFunc, Interaction::Rules::StaticRuleset::Type::global, whoInteractsDesc);
}

void Debug::message(Interaction::Context const& context, double** /*slf*/, double** /*otr*/, Interaction::GlobalValueCopy const& /*global*/) {
    Global::capture().log.println("[DEBUG] Static ruleset '::message' invoked! Domain ID: ", context.self.getId());
}

void Debug::error(Interaction::Context const& context, double** /*slf*/, double** /*otr*/, Interaction::GlobalValueCopy const& /*global*/) {
    Global::capture().log.println("[DEBUG] Static ruleset '::error' invoked! Domain ID: ", context.self.getId());
}

void Debug::whoInteracts(Interaction::Context const& context, double** /*slf*/, double** /*otr*/, Interaction::GlobalValueCopy const& /*global*/) {
    Global::capture().log.println("[DEBUG] Static ruleset '::whoInteracts' invoked! Domain self ID: ", context.self.getId(), ", Domain other ID: ", context.other.getId());
}

} // namespace Nebulite::Module::Ruleset
