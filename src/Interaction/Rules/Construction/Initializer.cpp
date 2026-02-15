#include "Interaction/Rules/Construction/Initializer.hpp"

//------------------------------------------
// Modules
#include "RulesetModule/Camera.hpp"
#include "RulesetModule/Debug.hpp"
#include "RulesetModule/Movement.hpp"
#include "RulesetModule/Physics.hpp"


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
