#include "Interaction/Rules/Construction/Initializer.hpp"

//------------------------------------------
// Modules
#include "Module/Ruleset/Camera.hpp"
#include "Module/Ruleset/Debug.hpp"
#include "Module/Ruleset/Movement.hpp"
#include "Module/Ruleset/Physics.hpp"


//------------------------------------------
// Shortcut
#define Module Nebulite::RulesetModule

//------------------------------------------
namespace Nebulite::Interaction::Rules::Construction {

void rulesetMapInit(StaticRulesetMap* srm) {
    static Module::Camera cameraModule;
    cameraModule.registerModule(*srm);
    static Module::Debug debugModule;
    debugModule.registerModule(*srm);
    static Module::Movement movementModule;
    movementModule.registerModule(*srm);
    static Module::Physics physicsModule;
    physicsModule.registerModule(*srm);
}

} // namespace Nebulite::Interaction::Rules::Construction
