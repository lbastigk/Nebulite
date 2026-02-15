#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "DomainModule/GlobalSpace/Time.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "RulesetModule/Movement.hpp"
#include "ScopeAccessor.hpp"

namespace Nebulite::RulesetModule {

Movement::Movement() : RulesetModule(moduleName) {
    // Global rulesets
    BIND_STATIC_ASSERT(RulesetType::Global, &Movement::clip, clipName, clipDesc);

    // Local rulesets

    // Global Variables
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.dt = Global::shareScopeBase(token).getStableDoublePointer(DomainModule::GlobalSpace::Time::Key::time_dt); // Simulation delta time
}

// Global rulesets

// TODO: Still rough collision handling, needs improvement...
void Movement::clip(Interaction::Context const& context) const {
    // Get ordered cache lists for both entities for base values
    double** slf = getBaseList(context.self, baseKeys);
    double** otr = getBaseList(context.other, baseKeys);

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
        Error::println("Circle-circle elastic clipping not yet implemented.");
    }
    else if (radius1 > 0.0) {
        Error::println("Circle-box elastic clipping not yet implemented.");
    }
    else if (radius2 > 0.0) {
        Error::println("Box-circle elastic clipping not yet implemented.");
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
            // Overlap checks for each axis + otr must be moving towards that axis
            //                    -> overlap next frame, ...                               but not currently
            bool const conditionX = !(p1X + size1X < nextP2X || nextP2X + size2X < p1X) && (p1X + size1X < p2X || p2X + size2X < p1X);
            bool const conditionY = !(p1Y + size1Y < nextP2Y || nextP2Y + size2Y < p1Y) && (p1Y + size1Y < p2Y || p2Y + size2Y < p1Y);


            double drX = 0.0;
            double drY = 0.0;

            // Determine drX and drY based on interpolation
            if (conditionX || conditionY) {
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
                if (v2X == 0.0) drX = 0.0;
                if (v2Y == 0.0) drY = 0.0;
            }

            // TODO: Allow for edge sliding:
            //       if otr is close to the edge of slf (maybe 1 pixel), allow for a correction dr along that axis
            //       to allow for sliding along the edge instead of a full stop
            //       This makes movement feel smoother and more natural, especially when moving along walls or other surfaces

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
            if (conditionX) {
                baseVal(otr, Key::physics_vX) = 0.0;
                baseVal(otr, Key::physics_FX) = 0.0;
            }
            if (conditionY) {
                baseVal(otr, Key::physics_vY) = 0.0;
                baseVal(otr, Key::physics_FY) = 0.0;
            }
        }
    }
}

} // namespace Nebulite::RulesetModule
