#include "Nebulite.hpp"
#include "RulesetModule/Movement.hpp"

#include "ScopeAccessor.hpp"
#include "Core/GlobalSpace.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

#include "DomainModule/GlobalSpace/Time.hpp"

namespace Nebulite::RulesetModule {

Movement::Movement() : RulesetModule(moduleName) {
    // Global rulesets
    BIND_STATIC_ASSERT(RulesetType::Global, &Movement::clip, clipName, clipDesc);

    // Local rulesets

    // Global Variables
    //auto const token = getRulesetModuleAccessToken(*this);
}

// Global rulesets

// TODO: Causes a one-frame delay in repositioning, we should use position prediction using velocity
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
        bool const baseCondition = p1X < p2X + size2X // right side overlap
            && p1Y < p2Y + size2Y // bottom side overlap
            && p2X < p1X + size1X // left side overlap
            && p2Y < p1Y + size1Y; // top side overlap

        //------------------------------------------
        // Potential collision response

        if (baseCondition) {
            // Overlap checks for each axis + otr must be moving towards that axis
            bool const conditionX = !(p1Y + size1Y - 2 < p2Y || p2Y + size2Y - 2 < p1Y) && v2X != 0.0;
            bool const conditionY = !(p1X + size1X - 2 < p2X || p2X + size2X - 2 < p1X) && v2Y != 0.0;

            if (conditionX) {
                // Reset position outside the other object
                auto slfLock = context.self.lockDocument();
                if (p1X < p2X) {
                    // self is to the left of other, meaning we set other to the right of self
                    baseVal(otr, Key::posX) = p1X + size2X;
                }
                else {
                    // self is to the right of other, meaning we set other to the left of self
                    baseVal(otr, Key::posX) = p1X - size1X;
                }
            }
            if (conditionY) {
                // Reset position outside the other object
                auto slfLock = context.self.lockDocument();
                if (p1Y < p2Y) {
                    // self is above other, meaning we set other to below self
                    baseVal(otr, Key::posY) = p1Y + size2Y;
                }
                else {
                    // self is below other, meaning we set other to above self
                    baseVal(otr, Key::posY) = p1Y - size1Y;
                }
            }
        }
    }
}

} // namespace Nebulite::RulesetModule
