#include "Interaction/Rules/RulesetModules/Physics.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Physics::gravity(Context const& context) {
    //----------------------------------------------
    // Workspace Setup

    // Variables from globalspace and constants
    static double const* G = Nebulite::global().getDoc()->getStableDoublePointer("physics.G");
    static double invMaxAccel = 1.0 / 1e0; // To prevent extreme accelerations

    //----------------------------------------------
    // Main Logic

    // Get ordered cache lists for both entities for base values
    double** slf = getBaseList(context.self);
    double** otr = getBaseList(context.other);

    // Calculate distance components
    double const distanceX = baseVal(slf,Key::posX) - baseVal(otr,Key::posX);
    double const distanceY = baseVal(slf,Key::posY) - baseVal(otr,Key::posY);

    // Avoid division by zero by adding a small epsilon
    double const denominator = std::pow((distanceX * distanceX + distanceY * distanceY),1.5) + 1; // +1 to avoid singularity

    // Calculate aX and aY of other
    auto otrLock = context.other.getDoc()->lock();
    baseVal(otr, Key::physics_aX) += *globalVal.G * baseVal(slf, Key::physics_mass) * distanceX / denominator;
    baseVal(otr, Key::physics_aY) += *globalVal.G * baseVal(slf, Key::physics_mass) * distanceY / denominator;
}

}