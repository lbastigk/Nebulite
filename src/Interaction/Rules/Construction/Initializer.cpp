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
    static Module::Ruleset::Camera const cameraModule;
    cameraModule.registerModule(*srm);
    static Module::Ruleset::Debug const debugModule;
    debugModule.registerModule(*srm);
    static Module::Ruleset::Movement const movementModule;
    movementModule.registerModule(*srm);
    static Module::Ruleset::Physics const physicsModule;
    physicsModule.registerModule(*srm);
}

} // namespace Nebulite::Interaction::Rules::Construction
