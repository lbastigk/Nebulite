#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/Time.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "RulesetModule/Movement.hpp"
#include "ScopeAccessor.hpp"

#include <cfloat>

namespace Nebulite::RulesetModule {

Movement::Movement() : RulesetModule(moduleName) {
    std::function<double**(const Interaction::Execution::Domain&)> const baseListFunc = [this](const Interaction::Execution::Domain& domain) -> double** {
        double** v;
        ensureBaseList(domain, baseKeys, v);
        return v;
    };

    // Global rulesets
    BIND_STATIC_ASSERT(RulesetType::Global, &Movement::clip, clipName, clipDesc, baseListFunc);

    // Local rulesets

    // Global Variables
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.dt = Global::shareScope(token).getStableDoublePointer(DomainModule::GlobalSpace::Time::Key::time_dt); // Simulation delta time
}

// Global rulesets

// TODO: Still rough collision handling, needs improvement...
void Movement::clip(Interaction::Context const& context, double**& slf, double**& otr) const {
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
    double const v2X = baseVal(otr, Key::physics_vX);
    double const v2Y = baseVal(otr, Key::physics_vY);
    double const m1 = baseVal(slf, Key::physics_mass);
    double const m2 = baseVal(otr, Key::physics_mass);

    // Estimate next position
    double const nextP2X = p2X + v2X * *globalVal.dt;
    double const nextP2Y = p2Y + v2Y * *globalVal.dt;

    // Prioritize circle collision if radius is set (> 0)
    if (radius1 > 0.0 && radius2 > 0.0) {
        // TODO: Circle-circle collision detection and response
        Global::capture().error.println("Circle-circle elastic clipping not yet implemented.");
    }
    else if (radius1 > 0.0) {
        Global::capture().error.println("Circle-box elastic clipping not yet implemented.");
    }
    else if (radius2 > 0.0) {
        Global::capture().error.println("Box-circle elastic clipping not yet implemented.");
    }
    else {
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

                // If velocity is zero, don't apply any movement along that axis
                //if (v2X == 0.0) drX = 0.0;
                //if (v2Y == 0.0) drY = 0.0;
            }

            // TODO: Sanity check: if still colliding after that correction, manually set dr outside the nearest sides

            // TODO: Allow for edge sliding:
            //       if otr is close to the edge of slf (maybe 1 pixel), allow for a correction dr along that axis
            //       to allow for sliding along the edge instead of a full stop
            //       This makes movement feel smoother and more natural, especially when moving along walls or other surfaces

            // Edge sliding
            static double edgeThreshold = 3.0; // Threshold distance to consider for edge sliding
            bool const slideUp = willCollideY && std::fabs(p2Y + size2Y - p1Y) <= edgeThreshold && std::fabs(v2X) > DBL_EPSILON && std::fabs(v2Y) < DBL_EPSILON;
            bool const slideDown = willCollideY && std::abs(p2Y - (p1Y + size1Y)) <= edgeThreshold && std::fabs(v2X) > DBL_EPSILON && std::fabs(v2Y) < DBL_EPSILON;
            bool const slideLeft = willCollideX && std::fabs(p2X + size2X - p1X) <= edgeThreshold && std::fabs(v2Y) > DBL_EPSILON && std::fabs(v2X) < DBL_EPSILON;
            bool const slideRight = willCollideX && std::fabs(p2X - (p1X + size1X)) <= edgeThreshold && std::fabs(v2Y) > DBL_EPSILON && std::fabs(v2X) < DBL_EPSILON;

            // Allow for sliding along edges if close enough to the edge and moving towards it
            // Bit too fast, needs tuning
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
}

} // namespace Nebulite::RulesetModule
