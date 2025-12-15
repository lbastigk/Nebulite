#include "Interaction/Rules/RulesetModules/Physics.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

// Global rulesets

// TODO: Improve collision detection and response
//       predictive future collision detection to avoid tunneling
//       perhaps better to do so in separate ruleset module
//       first, call physics::predictCollision to see first future collision time
//       then, use that info here to properly interpolate position
//       that happens between now and the predicted next frame-time
// TODO: Add a repositioning step to resolve overlaps
void Physics::elasticCollision(ContextBase const& context) {
    // Special keys for this ruleset only
    static std::string lastCollisionX = "physics.collision.time.lastX";
    static std::string lastCollisionY = "physics.collision.time.lastY";

    // Get ordered cache lists for both entities for base values
    double** slf = getBaseList(context.self);
    double** otr = getBaseList(context.other);

    //------------------------------------------
    // Base condition check

    // Required values
    double const p1X = baseVal(slf, Key::posX);
    double const p1Y = baseVal(slf, Key::posY);
    double const p2X = baseVal(otr, Key::posX);
    double const p2Y = baseVal(otr, Key::posY);
    double const size1X = baseVal(slf, Key::spriteSizeX);
    double const size1Y = baseVal(slf, Key::spriteSizeY);
    double const size2X = baseVal(otr, Key::spriteSizeX);
    double const size2Y = baseVal(otr, Key::spriteSizeY);
    double const m1 = baseVal(slf, Key::physics_mass);
    double const m2 = baseVal(otr, Key::physics_mass);

    // Base overlap condition
    bool baseCondition = m1 > 0.0 && m2 > 0.0
                         && p1X < p2X + size2X // right side overlap
                         && p1Y < p2Y + size2Y // bottom side overlap
                         && p2X < p1X + size1X // left side overlap
                         && p2Y < p1Y + size1Y; // top side overlap

    //------------------------------------------
    // Potential collision response

    if (baseCondition) {
        // Overlap checks for each axis (?)
        bool conditionX = baseCondition && !(p1Y + size1Y - 2 < p2Y || p2Y + size2Y - 2 < p1Y);
        bool conditionY = baseCondition && !(p1X + size1X - 2 < p2X || p2X + size2X - 2 < p1X);

        // m1*v1 + m2*v2 = m1*v1new + m2*v2new
        // Split into v1new and v2new equations
        // v1new = ( (m1 - m2)*v1 + 2*m2*v2 ) / m
        // v2new = ( (m2 - m1)*v2 + 2*m1*v1 ) / m
        // Work backwards to get Forces:
        // F = m * dv / dt
        // F1 = m1 * (v1new - v1) / dt
        // F2 = m2 * (v2new - v2) / dt

        // Lock and write forces to other entity
        // For self to be affected, other needs to broadcast this ruleset as well
        // Under normal circumstances, only one condition should be true at a time
        // Meaning we don't have to optimize in a way to avoid locking twice
        // We can only check collision time after locking, otherwise it may be overwritten by another thread
        if (conditionX) {
            // Start Velocities
            double const v1X = baseVal(slf, Key::physics_vX);
            double const v2X = baseVal(otr, Key::physics_vX);

            // Calculate new velocities after collision
            double const v2newX = ((m2 - m1) * v2X + 2 * m1 * v1X) / (m1 + m2);

            // Translate velocity change to forces
            double const dt = *globalVal.dt;
            double const dF2X = m2 * (v2newX - v2X) / dt;

            // Get last collision time pointer
            double* lastColX = context.other.getDoc()->getStableDoublePointer(lastCollisionX);

            // Lock and write
            auto slfLock = context.self.getDoc()->lock();
            if (*lastColX < *globalVal.t) {
                baseVal(otr, Key::physics_FX) += dF2X;
                *lastColX = *globalVal.t;
            }
        }
        if (conditionY) {
            // Start Velocities
            double const v1Y = baseVal(slf, Key::physics_vY);
            double const v2Y = baseVal(otr, Key::physics_vY);

            // Calculate new velocity after collision
            double const v2newY = ((m2 - m1) * v2Y + 2 * m1 * v1Y) / (m1 + m2);

            // Translate velocity change to forces
            double const dt = *globalVal.dt;
            double const dF2Y = m2 * (v2newY - v2Y) / dt;

            // Get last collision time pointer
            double* lastColY = context.other.getDoc()->getStableDoublePointer(lastCollisionY);

            // Lock and write
            auto slfLock = context.self.getDoc()->lock();
            if (*lastColY < *globalVal.t) {
                baseVal(otr, Key::physics_FY) += dF2Y;
                *lastColY = *globalVal.t;
            }
        }
    }
}

void Physics::gravity(ContextBase const& context) {
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

// Local rulesets

void Physics::applyForce(ContextBase const& context) {
    // Get ordered cache list for self entity for base values
    double** slf = getBaseList(context.self);

    // Pre-calculate values before locking
    double const dt = *globalVal.dt;
    double const invMass = 1.0 / baseVal(slf, Key::physics_mass);
    double const aX = baseVal(slf, Key::physics_FX) * invMass;
    double const aY = baseVal(slf, Key::physics_FY) * invMass;
    double const dvX = aX * dt;
    double const dvY = aY * dt;

    // Lock and apply all physics calculations
    auto slfLock = context.self.getDoc()->lock();

    // Acceleration is based on F
    baseVal(slf, Key::physics_aX) = aX;
    baseVal(slf, Key::physics_aY) = aY;

    // Velocity and Position is based on integration of Acceleration over dt
    baseVal(slf, Key::physics_vX) += dvX;
    baseVal(slf, Key::physics_vY) += dvY;
    baseVal(slf, Key::posX) += baseVal(slf, Key::physics_vX) * dt;
    baseVal(slf, Key::posY) += baseVal(slf, Key::physics_vY) * dt;

    // Force reset after application
    baseVal(slf, Key::physics_FX) = 0.0;
    baseVal(slf, Key::physics_FY) = 0.0;
}

void Physics::drag(ContextBase const& context) {
    // Get ordered cache list for self entity for base values
    double** slf = getBaseList(context.self);

    // Drag coefficient (tunable parameter)
    static constexpr double dragCoefficient = 0.1;

    // Pre-calculate drag forces before locking
    double const vX = baseVal(slf, Key::physics_vX);
    double const vY = baseVal(slf, Key::physics_vY);
    double const dragForceX = -dragCoefficient * vX;
    double const dragForceY = -dragCoefficient * vY;

    // Lock and apply drag forces
    auto slfLock = context.self.getDoc()->lock();
    baseVal(slf, Key::physics_FX) += dragForceX;
    baseVal(slf, Key::physics_FY) += dragForceY;
}

} // namespace Nebulite::Interaction::Rules::RulesetModules