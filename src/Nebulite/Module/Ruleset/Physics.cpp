//------------------------------------------
// Includes

// Standard Library
#include <cassert>
#include <cmath>
#include <stdexcept>

// Nebulite
#include "Nebulite/Core/GlobalSpace.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite/Math/Equality.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Physics.hpp"
#include "Nebulite/Module/Domain/GlobalSpace/Time.hpp"
#include "Nebulite/Module/Ruleset/Physics.hpp"
#include "Nebulite/Nebulite.hpp"
#include "Nebulite/ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite::Module::Ruleset {

Physics::Physics() : RulesetModule(moduleName, this) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Global rulesets
    bind<elasticCollisionName>(&Physics::elasticCollision, baseListFunc, Interaction::Rules::StaticRuleset::Type::global, elasticCollisionDesc);
    bind<gravityName>(&Physics::gravity, baseListFunc, Interaction::Rules::StaticRuleset::Type::global, gravityDesc);

    // Local rulesets
    bind<storeLastPositionName>(&Physics::storeLastPosition, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, storeLastPositionDesc);
    bind<applyForceName>(&Physics::applyForce, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, applyForceDesc);
    bind<applyCorrectionName>(&Physics::applyCorrection, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, applyCorrectionDesc);
    bind<dragName>(&Physics::drag, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, dragDesc);

    // Global Variables
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.G = Global::shareScope(token).getStableDoublePointer(Domain::GlobalSpace::Physics::Key::Global::G); // Gravitational constant
    globalVal.dt = Global::shareScope(token).getStableDoublePointer(Domain::GlobalSpace::Time::Key::deltaTime); // Simulation delta time
    globalVal.t = Global::shareScope(token).getStableDoublePointer(Domain::GlobalSpace::Time::Key::time); // Simulation time
}

// Global rulesets

// TODO: add collision for circle-box and circle-circle
void Physics::elasticCollision(Interaction::Context const& context, double** slf, double** otr) const {
    if (context.global.getId() != Global::instance().getId()) {
        throw std::runtime_error("The global context must be the actual GlobalSpace, as this Ruleset relies on global variables!");
    }

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
    double const m1 = baseVal(slf, Key::physicsMass);
    double const m2 = baseVal(otr, Key::physicsMass);

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
            bool const conditionX = p1Y + size1Y - 2 >= p2Y && p2Y + size2Y - 2 >= p1Y;
            bool const conditionY = p1X + size1X - 2 >= p2X && p2X + size2X - 2 >= p1X;

            // m1*v1 + m2*v2 = m1*v1new + m2*v2New
            // Split into v1new and v2New equations
            // v1new = ( (m1 - m2)*v1 + 2*m2*v2 ) / m
            // v2New = ( (m2 - m1)*v2 + 2*m1*v1 ) / m
            // Work backwards to get Forces:
            // F = m * dv / dt
            // F1 = m1 * (v1new - v1) / dt
            // F2 = m2 * (v2New - v2) / dt

            // Lock and write forces to other entity
            // For self to be affected, other needs to broadcast this ruleset as well
            // Under normal circumstances, only one condition should be true at a time
            // Meaning we don't have to optimize in a way to avoid locking twice
            // We can only check collision time after locking, otherwise it may be overwritten by another thread
            if (conditionX) {
                // Start Velocities
                double const v1X = baseVal(slf, Key::physicsVelocityX);
                double const v2X = baseVal(otr, Key::physicsVelocityX);

                // Calculate new velocities after collision
                double const v2NewX = ((m2 - m1) * v2X + 2 * m1 * v1X) / (m1 + m2);

                // Lock and write
                auto slfLock = context.other.lockDocument();
                if (baseVal(otr, Key::physicsLastCollisionX) < *globalVal.t) {
                    baseVal(otr, Key::physicsCorrectionVelocityX) += v2NewX - v2X;
                    baseVal(otr, Key::physicsLastCollisionX) = *globalVal.t;
                }
            }
            if (conditionY) {
                // Start Velocities
                double const v1Y = baseVal(slf, Key::physicsVelocityY);
                double const v2Y = baseVal(otr, Key::physicsVelocityY);

                // Calculate new velocity after collision
                double const v2NewY = ((m2 - m1) * v2Y + 2 * m1 * v1Y) / (m1 + m2);

                // Lock and write
                auto slfLock = context.other.lockDocument();
                if (baseVal(otr, Key::physicsLastCollisionY) < *globalVal.t) {
                    baseVal(otr, Key::physicsCorrectionVelocityY) += v2NewY - v2Y;
                    baseVal(otr, Key::physicsLastCollisionY) = *globalVal.t;
                }
            }
        }
    }
}


void Physics::gravity(Interaction::Context const& context, double** slf, double** otr) const {
    assert(isGlobalContextCorrect(context));

    double const dx = baseVal(slf, Key::posX) - baseVal(otr, Key::posX);
    double const dy = baseVal(slf, Key::posY) - baseVal(otr, Key::posY);
    double const r2 = dx*dx + dy*dy + 1.0;   // softening
    double const invR = 1.0 / std::sqrt(r2);
    double const invR3 = invR * invR * invR;
    double const coeff = *globalVal.G * baseVal(slf, Key::physicsMass) * baseVal(otr, Key::physicsMass) * invR3;

    auto otrLock = context.other.lockDocument();
    baseVal(otr, Key::physicsForceX) += dx * coeff;
    baseVal(otr, Key::physicsForceY) += dy * coeff;
}

// Local rulesets

// NOLINTNEXTLINE
void Physics::storeLastPosition(Interaction::Context const& /*context*/, double** slf, double** /*otr*/) const {
    baseVal(slf, Key::physicsLastPositionX) = baseVal(slf, Key::posX);
    baseVal(slf, Key::physicsLastPositionY) = baseVal(slf, Key::posY);
}

void Physics::applyForce([[maybe_unused]] Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));

    // Pre-calculate values before locking
    double const dt = *globalVal.dt;
    double const invMass = 1.0 / baseVal(slf, Key::physicsMass);
    double const aX = baseVal(slf, Key::physicsForceX) * invMass;
    double const aY = baseVal(slf, Key::physicsForceY) * invMass;
    double const dvX = aX * dt;
    double const dvY = aY * dt;

    // Lock and apply all physics calculations
    // Local ruleset, no locking needed
    //auto slfLock = context.self.lockDocument();

    // Acceleration is based on F
    baseVal(slf, Key::physicsAccelerationX) = aX;
    baseVal(slf, Key::physicsAccelerationY) = aY;

    // Velocity and Position is based on integration of Acceleration over dt
    baseVal(slf, Key::physicsVelocityX) += dvX;
    baseVal(slf, Key::physicsVelocityY) += dvY;
    baseVal(slf, Key::posX) += baseVal(slf, Key::physicsVelocityX) * dt;
    baseVal(slf, Key::posY) += baseVal(slf, Key::physicsVelocityY) * dt;

    // Force reset after application
    baseVal(slf, Key::physicsForceX) = 0.0;
    baseVal(slf, Key::physicsForceY) = 0.0;
}

// NOLINTNEXTLINE
void Physics::applyCorrection(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    // Check if any corrections are significant enough to apply (greater than a small epsilon)
    if (!Math::isZero(baseVal(slf, Key::physicsCorrectionX))  || !Math::isZero(baseVal(slf, Key::physicsCorrectionY))
     || !Math::isZero(baseVal(slf, Key::physicsCorrectionVelocityX)) || !Math::isZero(baseVal(slf, Key::physicsCorrectionVelocityY))) {
        // Lock and apply corrections
        auto slfLock = context.self.lockDocument();
        baseVal(slf, Key::posX) += baseVal(slf, Key::physicsCorrectionX);
        baseVal(slf, Key::posY) += baseVal(slf, Key::physicsCorrectionY);
        baseVal(slf, Key::physicsVelocityX) += baseVal(slf, Key::physicsCorrectionVelocityX);
        baseVal(slf, Key::physicsVelocityY) += baseVal(slf, Key::physicsCorrectionVelocityY);

        // Reset corrections after application
        baseVal(slf, Key::physicsCorrectionX) = 0.0;
        baseVal(slf, Key::physicsCorrectionY) = 0.0;
        baseVal(slf, Key::physicsCorrectionVelocityX) = 0.0;
        baseVal(slf, Key::physicsCorrectionVelocityY) = 0.0;
    }
}

// NOLINTNEXTLINE
void Physics::drag(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    // Drag coefficient (tunable parameter)
    static constexpr double dragCoefficient = 0.1;

    // Pre-calculate drag forces before locking
    double const vX = baseVal(slf, Key::physicsVelocityX);
    double const vY = baseVal(slf, Key::physicsVelocityY);
    double const dragForceX = -dragCoefficient * vX;
    double const dragForceY = -dragCoefficient * vY;

    // Lock and apply drag forces
    auto slfLock = context.self.lockDocument();
    baseVal(slf, Key::physicsForceX) += dragForceX;
    baseVal(slf, Key::physicsForceY) += dragForceY;
}

} // namespace Nebulite::Module::Ruleset
