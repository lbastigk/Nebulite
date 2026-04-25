#include "Interaction/Rules/Construction/Initializer.hpp"

//------------------------------------------
// Modules
#include "Module/Ruleset/Camera.hpp"
#include "Module/Ruleset/Debug.hpp"
#include "Module/Ruleset/Movement.hpp"
#include "Module/Ruleset/Physics.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules::Construction {

void rulesetMapInit(StaticRulesetMap* srm) {
    static Module::Ruleset::Camera cameraModule;
    cameraModule.registerModule(*srm);
    static Module::Ruleset::Debug debugModule;
    debugModule.registerModule(*srm);
    static Module::Ruleset::Movement movementModule;
    movementModule.registerModule(*srm);
    static Module::Ruleset::Physics physicsModule;
    physicsModule.registerModule(*srm);
}

} // namespace Nebulite::Interaction::Rules::Construction
