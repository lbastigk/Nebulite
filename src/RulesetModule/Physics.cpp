//------------------------------------------
// Includes

// Standard Library
#include <cfloat>

// Nebulite
#include "Nebulite.hpp"
#include "ScopeAccessor.hpp"
#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/Time.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "RulesetModule/Physics.hpp"

namespace Nebulite::RulesetModule {

Physics::Physics() : RulesetModule(moduleName) {
    std::function<double**(const Interaction::Execution::Domain&)> const baseListFunc = [this](const Interaction::Execution::Domain& domain) -> double** {
        double** v;
        ensureBaseList(domain, baseKeys, v);
        return v;
    };

    // Global rulesets
    BIND_STATIC_ASSERT(RulesetType::Global, &Physics::elasticCollision, elasticCollisionName, elasticCollisionDesc, baseListFunc);
    BIND_STATIC_ASSERT(RulesetType::Global, &Physics::gravity, gravityName, gravityDesc, baseListFunc);

    // Local rulesets
    BIND_STATIC_ASSERT(RulesetType::Local, &Physics::applyForce, applyForceName, applyForceDesc, baseListFunc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Physics::applyCorrection, applyCorrectionName, applyCorrectionDesc, baseListFunc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Physics::drag, dragName, dragDesc, baseListFunc);

    // Global Variables
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.G = Global::shareScopeBase(token).getStableDoublePointer(DomainModule::GlobalSpace::Physics::Key::Global::G); // Gravitational constant
    globalVal.dt = Global::shareScopeBase(token).getStableDoublePointer(DomainModule::GlobalSpace::Time::Key::time_dt); // Simulation delta time
    globalVal.t = Global::shareScopeBase(token).getStableDoublePointer(DomainModule::GlobalSpace::Time::Key::time_t); // Simulation time
}

// Global rulesets

// TODO: add collision for circle-box and circle-circle
void Physics::elasticCollision(Interaction::Context const& context, double**& slf, double**& otr) const {
    //------------------------------------------
    // Base condition check

    // Required values
    double const radius1 = baseVal(slf, Key::sizeR);
    double const radius2 = baseVal(otr, Key::sizeR);
    double const p1X = baseVal(slf, Key::posX);
    double const p1Y = baseVal(slf, Key::posY);
    double const p2X = baseVal(otr, Key::posX);
    double const p2Y = baseVal(otr, Key::posY);
    double const size1X = baseVal(slf, Key::sizeX);
    double const size1Y = baseVal(slf, Key::sizeY);
    double const size2X = baseVal(otr, Key::sizeX);
    double const size2Y = baseVal(otr, Key::sizeY);
    double const m1 = baseVal(slf, Key::physics_mass);
    double const m2 = baseVal(otr, Key::physics_mass);

    // Prioritize circle collision if radius is set (> 0)
    if (radius1 > 0.0 && radius2 > 0.0) {
        // TODO: Circle-circle collision detection and response
        Global::capture().error.println("Circle-circle elastic collision not yet implemented.");
    }
    else if (radius1 > 0.0) {
        Global::capture().error.println("Circle-box elastic collision not yet implemented.");
    }
    else if (radius2 > 0.0) {
        Global::capture().error.println("Box-circle elastic collision not yet implemented.");
    }
    else {
        // Base overlap condition
        bool const baseCondition = m1 > 0.0 && m2 > 0.0
            && p1X < p2X + size2X // right side overlap
            && p1Y < p2Y + size2Y // bottom side overlap
            && p2X < p1X + size1X // left side overlap
            && p2Y < p1Y + size1Y; // top side overlap

        //------------------------------------------
        // Potential collision response

        if (baseCondition) {
            // Overlap checks for each axis (?)
            bool const conditionX = !(p1Y + size1Y - 2 < p2Y || p2Y + size2Y - 2 < p1Y);
            bool const conditionY = !(p1X + size1X - 2 < p2X || p2X + size2X - 2 < p1X);

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

                // Lock and write
                auto slfLock = context.other.lockDocument();
                if (baseVal(otr, Key::physics_lastCollisionX) < *globalVal.t) {
                    baseVal(otr, Key::physics_correction_vX) += v2newX - v2X;
                    baseVal(otr, Key::physics_lastCollisionX) = *globalVal.t;
                }
            }
            if (conditionY) {
                // Start Velocities
                double const v1Y = baseVal(slf, Key::physics_vY);
                double const v2Y = baseVal(otr, Key::physics_vY);

                // Calculate new velocity after collision
                double const v2newY = ((m2 - m1) * v2Y + 2 * m1 * v1Y) / (m1 + m2);

                // Lock and write
                auto slfLock = context.other.lockDocument();
                if (baseVal(otr, Key::physics_lastCollisionY) < *globalVal.t) {
                    baseVal(otr, Key::physics_correction_vY) += v2newY - v2Y;
                    baseVal(otr, Key::physics_lastCollisionY) = *globalVal.t;
                }
            }
        }
    }
}


void Physics::gravity(Interaction::Context const& context, double**& slf, double**& otr) const {
    double const dx = baseVal(slf, Key::posX) - baseVal(otr, Key::posX);
    double const dy = baseVal(slf, Key::posY) - baseVal(otr, Key::posY);

    double const r2 = dx*dx + dy*dy + 1.0;   // softening
    double const invR = 1.0 / std::sqrt(r2);
    double const invR3 = invR * invR * invR;

    double const G  = *globalVal.G;
    double const m1 = baseVal(slf, Key::physics_mass);
    double const m2 = baseVal(otr, Key::physics_mass);

    double const coeff = G * m1 * m2 * invR3;

    auto otrLock = context.other.lockDocument();

    baseVal(otr, Key::physics_FX) += dx * coeff;
    baseVal(otr, Key::physics_FY) += dy * coeff;
}

// Local rulesets

void Physics::applyForce(Interaction::Context const& /*context*/, double**& slf, double**&) const {
    // Pre-calculate values before locking
    double const dt = *globalVal.dt;
    double const invMass = 1.0 / baseVal(slf, Key::physics_mass);
    double const aX = baseVal(slf, Key::physics_FX) * invMass;
    double const aY = baseVal(slf, Key::physics_FY) * invMass;
    double const dvX = aX * dt;
    double const dvY = aY * dt;

    // Lock and apply all physics calculations
    // Local ruleset, no locking needed
    //auto slfLock = context.self.lockDocument();

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

// NOLINTNEXTLINE
void Physics::applyCorrection(Interaction::Context const& context, double**& slf, double**&) const {
    // Check if any corrections are significant enough to apply (greater than a small epsilon)
    if (std::abs(baseVal(slf, Key::physics_correction_X))  > DBL_EPSILON || std::abs(baseVal(slf, Key::physics_correction_Y))  > DBL_EPSILON
     || std::abs(baseVal(slf, Key::physics_correction_vX)) > DBL_EPSILON || std::abs(baseVal(slf, Key::physics_correction_vY)) > DBL_EPSILON) {
        // Lock and apply corrections
        auto slfLock = context.self.lockDocument();
        baseVal(slf, Key::posX) += baseVal(slf, Key::physics_correction_X);
        baseVal(slf, Key::posY) += baseVal(slf, Key::physics_correction_Y);
        baseVal(slf, Key::physics_vX) += baseVal(slf, Key::physics_correction_vX);
        baseVal(slf, Key::physics_vY) += baseVal(slf, Key::physics_correction_vY);

        // Reset corrections after application
        baseVal(slf, Key::physics_correction_X) = 0.0;
        baseVal(slf, Key::physics_correction_Y) = 0.0;
        baseVal(slf, Key::physics_correction_vX) = 0.0;
        baseVal(slf, Key::physics_correction_vY) = 0.0;
    }
}

// NOLINTNEXTLINE
void Physics::drag(Interaction::Context const& context, double**& slf, double**&) const {
    // Drag coefficient (tunable parameter)
    static constexpr double dragCoefficient = 0.1;

    // Pre-calculate drag forces before locking
    double const vX = baseVal(slf, Key::physics_vX);
    double const vY = baseVal(slf, Key::physics_vY);
    double const dragForceX = -dragCoefficient * vX;
    double const dragForceY = -dragCoefficient * vY;

    // Lock and apply drag forces
    auto slfLock = context.self.lockDocument();
    baseVal(slf, Key::physics_FX) += dragForceX;
    baseVal(slf, Key::physics_FY) += dragForceY;
}

} // namespace Nebulite::RulesetModule
