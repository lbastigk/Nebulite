#include "Interaction/Rules/RulesetMapInitializer.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

//------------------------------------------
// Modules
#include "Interaction/Rules/RulesetModules/Camera.hpp"
#include "Interaction/Rules/RulesetModules/Debug.hpp"
#include "Interaction/Rules/RulesetModules/Physics.hpp"


//------------------------------------------
// Shortcut
#define Module Nebulite::Interaction::Rules::RulesetModules

//------------------------------------------
namespace Nebulite::Interaction::Rules {
void rulesetMapInit(StaticRulesetMap* srm) {
    // Register static rulesets from various modules here
    // Example:
    // Nebulite::Interaction::Rules::RulesetModules::Physics physicsModule;
    // physicsModule.registerModule(srm);
    static Module::Camera cameraModule;
    cameraModule.registerModule(*srm);
    static Module::Debug debugModule;
    debugModule.registerModule(*srm);
    static Module::Physics physicsModule;
    physicsModule.registerModule(*srm);
}
}