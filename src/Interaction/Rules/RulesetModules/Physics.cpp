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
    double const coefficient = *globalVal.G * baseVal(slf, Key::physics_mass) * baseVal(otr, Key::physics_mass) / denominator;

    // Apply gravitational force to other entity
    auto otrLock = context.other.getDoc()->lock();
    baseVal(otr, Key::physics_FX) += distanceX * coefficient;
    baseVal(otr, Key::physics_FY) += distanceY * coefficient;
}

void Physics::applyForce(Context const& context) {
    // Get ordered cache list for self entity for base values
    double** slf = getBaseList(context.self);

    // Lock and apply all physics calculations
    auto slfLock = context.self.getDoc()->lock();

    // Acceleration is based on F
    baseVal(slf, Key::physics_aX) = baseVal(slf, Key::physics_FX) / baseVal(slf, Key::physics_mass);
    baseVal(slf, Key::physics_aY) = baseVal(slf, Key::physics_FY) / baseVal(slf, Key::physics_mass);

    // Velocity and Position is based on integration of Acceleration over dt
    baseVal(slf, Key::physics_vX) += baseVal(slf, Key::physics_aX) * (*globalVal.dt);
    baseVal(slf, Key::physics_vY) += baseVal(slf, Key::physics_aY) * (*globalVal.dt);
    baseVal(slf, Key::posX) += baseVal(slf, Key::physics_vX) * (*globalVal.dt);
    baseVal(slf, Key::posY) += baseVal(slf, Key::physics_vY) * (*globalVal.dt);

    // Force reset after application
    baseVal(slf, Key::physics_FX) = 0.0;
    baseVal(slf, Key::physics_FY) = 0.0;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules