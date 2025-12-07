#include "Interaction/Rules/RulesetModules/Physics.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Physics::gravity(Context const& context) {
    // Get ordered cache lists for both entities for base values
    double** slf = getBaseList(context.self);
    double** otr = getBaseList(context.other);

    // Calculate distance components
    double const distanceX = baseVal(slf, Key::posX) - baseVal(otr, Key::posX);
    double const distanceY = baseVal(slf, Key::posY) - baseVal(otr, Key::posY);

    // Avoid division by zero by adding a small epsilon
    double const denominator = std::pow((distanceX * distanceX + distanceY * distanceY), 1.5) + 1; // +1 to avoid singularity

    // Calculate aX and aY of other
    auto otrLock = context.other.getDoc()->lock();
    baseVal(otr, Key::physics_aX) += *globalVal.G * baseVal(slf, Key::physics_mass) * distanceX / denominator;
    baseVal(otr, Key::physics_aY) += *globalVal.G * baseVal(slf, Key::physics_mass) * distanceY / denominator;
}

void Physics::applyForce(Context const& context) {
    // Get ordered cache list for self entity for base values
    double** slf = getBaseList(context.self);

    // Lock and apply all physics calculations
    auto slfLock = context.self.getDoc()->lock();
    baseVal(slf, Key::physics_aX) += baseVal(slf, Key::physics_FX) / baseVal(slf, Key::physics_mass);
    baseVal(slf, Key::physics_aY) += baseVal(slf, Key::physics_FY) / baseVal(slf, Key::physics_mass);
    baseVal(slf, Key::physics_vX) += baseVal(slf, Key::physics_aX) * (*globalVal.dt);
    baseVal(slf, Key::physics_vY) += baseVal(slf, Key::physics_aY) * (*globalVal.dt);
    baseVal(slf, Key::posX) += baseVal(slf, Key::physics_vX) * (*globalVal.dt);
    baseVal(slf, Key::posY) += baseVal(slf, Key::physics_vY) * (*globalVal.dt);

    // Force reset after application
    baseVal(slf, Key::physics_FX) = 0.0;
    baseVal(slf, Key::physics_FY) = 0.0;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules