#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/Time.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "Math/Equality.hpp"
#include "RulesetModule/Movement.hpp"
#include "ScopeAccessor.hpp"

#include <cfloat>

namespace Nebulite::RulesetModule {

Movement::Movement() : RulesetModule(moduleName) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Global rulesets
    bind<clipName>(RulesetType::Global, &Movement::clip, clipDesc, baseListFunc);
    bind<boxEdgeSlidingName>(RulesetType::Global, &Movement::boxEdgeSliding, boxEdgeSlidingDesc, baseListFunc);

    // Local rulesets

    // Global Variables
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.dt = Global::shareScope(token).getStableDoublePointer(DomainModule::GlobalSpace::Time::Key::time_dt); // Simulation delta time
}

// Global rulesets

void Movement::collisionCircleCircle(Interaction::Context const& context, double**& slf, double**& otr) const {
    Global::capture().error.println("Circle-circle elastic clipping not yet implemented. Skipping ruleset.");
    (void) *globalVal.dt; // Avoid unused variable warning for now, will be used in future implementation
    (void) context;
    (void) slf;
    (void) otr;
}

void Movement::collisionCircleBox(Interaction::Context const& context, double**& slf, double**& otr) const {
    Global::capture().error.println("Circle-box elastic clipping not yet implemented. Skipping ruleset.");
    (void) *globalVal.dt; // Avoid unused variable warning for now, will be used in future implementation
    (void) context;
    (void) slf;
    (void) otr;
}

void Movement::collisionBoxCircle(Interaction::Context const& context, double**& slf, double**& otr) const {
    Global::capture().error.println("Box-circle elastic clipping not yet implemented. Skipping ruleset.");
    (void) *globalVal.dt; // Avoid unused variable warning for now, will be used in future implementation
    (void) context;
    (void) slf;
    (void) otr;
}

// TODO: Still rough collision handling, needs improvement...
void Movement::collisionBoxBox(Interaction::Context const& context, double**& slf, double**& otr) const {
    double const& p1X = baseVal(slf, Key::posX);
    double const& p1Y = baseVal(slf, Key::posY);
    double const& p2X = baseVal(otr, Key::posX);
    double const& p2Y = baseVal(otr, Key::posY);
    double const& size1X = baseVal(slf, Key::sizeX);
    double const& size1Y = baseVal(slf, Key::sizeY);
    double const& size2X = baseVal(otr, Key::sizeX);
    double const& size2Y = baseVal(otr, Key::sizeY);
    double const& v2X = baseVal(otr, Key::physics_vX);
    double const& v2Y = baseVal(otr, Key::physics_vY);
    double const& m1 = baseVal(slf, Key::physics_mass);
    double const& m2 = baseVal(otr, Key::physics_mass);

    // Estimate next position
    double const& nextP2X = p2X + v2X * *globalVal.dt;
    double const& nextP2Y = p2Y + v2Y * *globalVal.dt;

    // Base overlap condition
    bool const baseCondition = m1 > 0.0 && m2 > 0.0 // Both objects must have mass to collide
        && p1X < nextP2X + size2X // right side overlap next frame
        && p1Y < nextP2Y + size2Y // bottom side overlap next frame
        && nextP2X < p1X + size1X // left side overlap next frame
        && nextP2Y < p1Y + size1Y; // top side overlap next frame

    //------------------------------------------
    // Potential collision response

    if (baseCondition) {
        // Overlap checks for each axis
        bool const willCollideX = !(p1X + size1X < nextP2X || nextP2X + size2X < p1X);
        bool const willCollideY = !(p1Y + size1Y < nextP2Y || nextP2Y + size2Y < p1Y);
        bool const isCollidingX = !(p1X + size1X < p2X || p2X + size2X < p1X);
        bool const isCollidingY = !(p1Y + size1Y < p2Y || p2Y + size2Y < p1Y);

        double drX = 0.0;
        double drY = 0.0;

        // Determine drX and drY based on interpolation
        if (willCollideX || willCollideY) {
            // Based on current velocity vector, determine corresponding dr direction that is outside the object

            if (p1X + size1X < nextP2X + drX) {
                drX = nextP2X + drX - (p1X + size1X);
            }
            else if (nextP2X + drX + size2X < p1X) {
                drX = nextP2X + drX + size2X - p1X;
            }
            if (p1Y + size1Y < nextP2Y + drY) {
                drY = nextP2Y + drY - (p1Y + size1Y);
            }
            else if (nextP2Y + drY + size2Y < p1Y) {
                drY = nextP2Y + drY + size2Y - p1Y;
            }
        }

        // TODO: Sanity check: if still colliding after that correction, manually set dr outside the nearest sides?

        // Set new values:
        auto slfLock = context.self.lockDocument();
        baseVal(otr, Key::posX) += drX; // Move other back along the x-axis until no longer colliding
        baseVal(otr, Key::posY) += drY; // Move other back along the y-axis until no longer colliding

        // Assumption: all box collisions are happening at integer positions, so we set posX and poY to the nearest integer
        if (drX < 0.0) {
            baseVal(otr, Key::posX) = std::floor(baseVal(otr, Key::posX));
        }
        else if (drX > 0.0) {
            baseVal(otr, Key::posX) = std::ceil(baseVal(otr, Key::posX));
        }
        if (drY < 0.0) {
            baseVal(otr, Key::posY) = std::floor(baseVal(otr, Key::posY));
        }
        else if (drY > 0.0) {
            baseVal(otr, Key::posY) = std::ceil(baseVal(otr, Key::posY));
        }

        // Stop movement along axes only if the collision is new (i.e. not currently overlapping along that axis)
        if (willCollideX && !isCollidingX) {
            baseVal(otr, Key::physics_vX) = 0.0;
            baseVal(otr, Key::physics_FX) = 0.0;
        }
        if (willCollideY && !isCollidingY) {
            baseVal(otr, Key::physics_vY) = 0.0;
            baseVal(otr, Key::physics_FY) = 0.0;
        }
    }
}

void Movement::clip(Interaction::Context const& context, double**& slf, double**& otr) const {
    // Assume context part is circle if radius is set (> 0)
    if (auto const& radius = Radius(slf, otr); radius.slf > 0.0 && radius.otr > 0.0) {
        collisionCircleCircle(context, slf, otr);
    }
    else if (radius.slf > 0.0) {
        collisionCircleBox(context, slf, otr);
    }
    else if (radius.otr > 0.0) {
        collisionBoxCircle(context, slf, otr);
    }
    else {
        collisionBoxBox(context, slf, otr);
    }
}

void Movement::boxEdgeSliding(Interaction::Context const& context, double**& slf, double**& otr) const{
    double const& p1X = baseVal(slf, Key::posX);
    double const& p1Y = baseVal(slf, Key::posY);
    double const& p2X = baseVal(otr, Key::posX);
    double const& p2Y = baseVal(otr, Key::posY);
    double const& size1X = baseVal(slf, Key::sizeX);
    double const& size1Y = baseVal(slf, Key::sizeY);
    double const& size2X = baseVal(otr, Key::sizeX);
    double const& size2Y = baseVal(otr, Key::sizeY);
    double const& v2X = baseVal(otr, Key::physics_vX);
    double const& v2Y = baseVal(otr, Key::physics_vY);
    double const& m1 = baseVal(slf, Key::physics_mass);
    double const& m2 = baseVal(otr, Key::physics_mass);

    // Estimate next position
    double const& nextP2X = p2X + v2X * *globalVal.dt;
    double const& nextP2Y = p2Y + v2Y * *globalVal.dt;

    // Base overlap condition
    bool const baseCondition = m1 > 0.0 && m2 > 0.0 // Both objects must have mass to collide
        && p1X < nextP2X + size2X // right side overlap next frame
        && p1Y < nextP2Y + size2Y // bottom side overlap next frame
        && nextP2X < p1X + size1X // left side overlap next frame
        && nextP2Y < p1Y + size1Y; // top side overlap next frame

    if (baseCondition) {
        // Overlap checks for each axis
        bool const willCollideX = !(p1X + size1X < nextP2X || nextP2X + size2X < p1X);
        bool const willCollideY = !(p1Y + size1Y < nextP2Y || nextP2Y + size2Y < p1Y);

        double drX = 0.0;
        double drY = 0.0;

        // Edge sliding
        static double edgeThreshold = 3.0; // Threshold distance to consider for edge sliding
        bool const slideUp = willCollideY && std::fabs(p2Y + size2Y - p1Y) <= edgeThreshold && !Math::isZero(v2X) && !Math::isZero(v2Y);
        bool const slideDown = willCollideY && std::abs(p2Y - (p1Y + size1Y)) <= edgeThreshold && !Math::isZero(v2X) && !Math::isZero(v2Y);
        bool const slideLeft = willCollideX && std::fabs(p2X + size2X - p1X) <= edgeThreshold && !Math::isZero(v2Y) && !Math::isZero(v2X);
        bool const slideRight = willCollideX && std::fabs(p2X - (p1X + size1X)) <= edgeThreshold && !Math::isZero(v2Y) && !Math::isZero(v2X);

        // Allow for sliding along edges if close enough to the edge and moving towards it
        if (slideUp && !slideDown) {
            drY -= 1;
        }
        if (slideDown && !slideUp) {
            drY += 1;
        }
        if (slideLeft && !slideRight) {
            drX -= 1;
        }
        if (slideRight && !slideLeft) {
            drX += 1;
        }

        // F = m*a = m*v/dt = m*dr/dt
        double const dFX = m2*drX / *globalVal.dt;
        double const dFY = m2*drY / *globalVal.dt;

        auto lock = context.other.lockDocument();
        baseVal(otr, Key::physics_FX) += dFX;
        baseVal(otr, Key::physics_FY) += dFY;
    }
}

} // namespace Nebulite::RulesetModule
