//------------------------------------------
// Includes

// Nebulite
#include "Nebulite/Interaction/Rules/Construction/Initializer.hpp"
#include "Nebulite/Module/Ruleset/Camera.hpp"
#include "Nebulite/Module/Ruleset/Debug.hpp"
#include "Nebulite/Module/Ruleset/Movement.hpp"
#include "Nebulite/Module/Ruleset/Physics.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules::Construction {

void rulesetMapInit(StaticRulesetMap* srm) {
    static const Module::Ruleset::Camera cameraModule;
    cameraModule.registerModule(*srm);
    static const Module::Ruleset::Debug debugModule;
    debugModule.registerModule(*srm);
    static const Module::Ruleset::Movement movementModule;
    movementModule.registerModule(*srm);
    static const Module::Ruleset::Physics physicsModule;
    physicsModule.registerModule(*srm);
}

} // namespace Nebulite::Interaction::Rules::Construction
