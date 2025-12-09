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

void Physics::elasticCollision(Context const& context) {
    // Special keys for this function
    std::string const lastCollisionX = "physics.collision.time.lastX";
    std::string const lastCollisionY = "physics.collision.time.lastY";

    // Get ordered cache lists for both entities for base values
    double** slf = getBaseList(context.self);
    double** otr = getBaseList(context.other);

    // Prerequisites
    // Objects boundaries would overlap on X-axis or Y-axis next frame
    // Both objects have non-zero mass
    // Perhaps some fancy interpolation to set the position
    // Ignore if velocity in examined axis is zero for this object
    // -> if no overlap right now, but next frame, then interpolate to the point of contact and set new positions
    //    dt = dt_before_contact + dt_after_contact
    //    rContact = r + v * dt_before_contact
    //    rNew = rContact + v_new * dt_after_contact
    // Set a flag that collision happened to avoid multiple collision responses in a single frame
    // This is an issue if the object collides with multiple objects in a single frame
    // e.g.: A stack of objects, or just corner collisions

    // m1*v1 + m2*v2 = m1*v1new + m2*v2new
    // Split into v1new and v2new equations
    // v1new = ( (m1 - m2)*v1 + 2*m2*v2 ) / m
    // v2new = ( (m2 - m1)*v2 + 2*m1*v1 ) / m
    // Work backwards to get Forces:
    // F = m * dv / dt
    // F1 = m1 * (v1new - v1) / dt
    // F2 = m2 * (v2new - v2) / dt

    double const m1 = baseVal(slf, Key::physics_mass);
    double const m2 = baseVal(otr, Key::physics_mass);
    double const m = m1 + m2;

    double const v1X = baseVal(slf, Key::physics_vX);
    double const v1Y = baseVal(slf, Key::physics_vY);
    double const v2X = baseVal(otr, Key::physics_vX);
    double const v2Y = baseVal(otr, Key::physics_vY);

    double const dt = *globalVal.dt;
    double const v2newX = ( (m2 - m1)*v2X + 2*m1*v1X ) / m;
    double const v2newY = ( (m2 - m1)*v2Y + 2*m1*v1Y ) / m;

    double const dF2X = m2 * (v2newX - v2X) / dt;
    double const dF2Y = m2 * (v2newY - v2Y) / dt;

    // Lock and write forces to other entity
    // For self to be affected, other needs to broadcast this ruleset as well
    bool conditionX = false;    // TODO
    bool conditionY = false;    // TODO

    // Under normal circumstances, only one condition should be true at a time
    // Meaning we don't have to optimize in a way to avoid double locking
    if (conditionX) {
        auto slfLock = context.self.getDoc()->lock();
        baseVal(slf, Key::physics_FX) -= dF2X;
    }
    if (conditionY) {
        auto slfLock = context.self.getDoc()->lock();
        baseVal(slf, Key::physics_FY) -= dF2Y;
    }
}

void Physics::applyForce(Context const& context) {
    // Get ordered cache list for self entity for base values
    double** slf = getBaseList(context.self);

    // Pre-calculate values before locking
    double const dt  = *globalVal.dt;
    double const ddt = dt * dt;
    double const invMass = 1.0 / baseVal(slf, Key::physics_mass);
    double const aX = baseVal(slf, Key::physics_FX) * invMass;
    double const aY = baseVal(slf, Key::physics_FY) * invMass;
    double const dvX = aX * dt;
    double const dvY = aY * dt;
    double const dPosX = aX * ddt;
    double const dPosY = aY * ddt;

    // Lock and apply all physics calculations
    auto slfLock = context.self.getDoc()->lock();

    // Acceleration is based on F
    baseVal(slf, Key::physics_aX) = aX;
    baseVal(slf, Key::physics_aY) = aY;

    // Velocity and Position is based on integration of Acceleration over dt
    baseVal(slf, Key::physics_vX) += dvX;
    baseVal(slf, Key::physics_vY) += dvY;
    baseVal(slf, Key::posX) += dPosX;
    baseVal(slf, Key::posY) += dPosY;

    // Force reset after application
    baseVal(slf, Key::physics_FX) = 0.0;
    baseVal(slf, Key::physics_FY) = 0.0;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules