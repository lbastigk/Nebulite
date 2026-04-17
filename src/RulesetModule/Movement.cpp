//------------------------------------------
// Includes

// Standard Library
#include <cfloat>

// Nebulite
#include "Nebulite.hpp"
#include "Core/GlobalSpace.hpp"
#include "Module/Domain/GlobalSpace/Time.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"
#include "RulesetModule/Movement.hpp"
#include "ScopeAccessor.hpp"

namespace Nebulite::RulesetModule {

Movement::Movement() : RulesetModule(moduleName) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Global rulesets
    bind<detectClippingName>(RulesetType::Global, &Movement::detectClipping, detectClippingDesc, baseListFunc);

    // Local rulesets
    bind<processClippingName>(RulesetType::Local, &Movement::processClipping, processClippingDesc, baseListFunc);

    // Global Variables
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.dt = Global::shareScope(token).getStableDoublePointer(DomainModule::GlobalSpace::Time::Key::time_dt); // Simulation delta time
}

// NOLINTNEXTLINE
void Movement::detectClipping(Interaction::Context const& context, double**& slf, double**& otr) const {
    // Assume context part is circle if radius is set (> 0)
    if (auto const& radius = Radius(slf, otr); radius.slf > 0.0 && radius.otr > 0.0) {
        // TODO
    }
    else if (radius.slf > 0.0) {
        // TODO
    }
    else if (radius.otr > 0.0) {
        // TODO
    }
    else {
        double const& p1X = baseVal(slf, Key::posX);
        double const& p1Y = baseVal(slf, Key::posY);
        double const& p2X = baseVal(otr, Key::posX);
        double const& p2Y = baseVal(otr, Key::posY);
        double const& size1X = baseVal(slf, Key::sizeX);
        double const& size1Y = baseVal(slf, Key::sizeY);
        double const& size2X = baseVal(otr, Key::sizeX);
        double const& size2Y = baseVal(otr, Key::sizeY);
        double const& m1 = baseVal(slf, Key::physics_mass);
        double const& m2 = baseVal(otr, Key::physics_mass);

        if (bool const baseCondition = m1 > 0.0 && m2 > 0.0; baseCondition) {
            // Determine if they align along an axis
            bool const axisAlignX = std::max(p1X, p2X) < std::min(p1X + size1X, p2X + size2X);
            bool const axisAlignY = std::max(p1Y, p2Y) < std::min(p1Y + size1Y, p2Y + size2Y);

            bool const isNorth = axisAlignX && (p1Y + size1Y <= p2Y);
            bool const isEast = axisAlignY && (p1X >= p2X + size2X);
            bool const isSouth = axisAlignX && (p1Y >= p2Y + size2Y);
            bool const isWest = axisAlignY && (p1X + size1X <= p2X);

            if (isNorth) {
                double const dist = p2Y - p1Y - size1Y;
                auto lock = context.other.lockDocument();
                if (double& closestCurrent = baseVal(otr, Key::clip_closest_N); closestCurrent > dist) {
                    closestCurrent = dist;
                }
            }
            if (isEast) {
                double const dist = p1X - p2X - size2X;
                auto lock = context.other.lockDocument();
                if (double& closestCurrent = baseVal(otr, Key::clip_closest_E); closestCurrent > dist) {
                    closestCurrent = dist;
                }
            }
            if (isSouth) {
                double const dist = p1Y - p2Y - size2Y;
                auto lock = context.other.lockDocument();
                if (double& closestCurrent = baseVal(otr, Key::clip_closest_S); closestCurrent > dist) {
                    closestCurrent = dist;
                }
            }
            if (isWest) {
                double const dist = p2X - p1X - size1X;
                auto lock = context.other.lockDocument();
                if (double& closestCurrent = baseVal(otr, Key::clip_closest_W); closestCurrent > dist) {
                    closestCurrent = dist;
                }
            }
        }
    }
}

// NOLINTNEXTLINE
void Movement::processClipping(Interaction::Context const& /*context*/, double**& slf, double**& /*otr*/) const {
    // Nearest corners
    double& N = baseVal(slf, Key::clip_closest_N);
    double& E = baseVal(slf, Key::clip_closest_E);
    double& S = baseVal(slf, Key::clip_closest_S);
    double& W = baseVal(slf, Key::clip_closest_W);

    double& X = baseVal(slf, Key::posX);
    double& Y = baseVal(slf, Key::posY);

    auto const [dX, dY] = std::make_pair(
        X - baseVal(slf, Key::position_last_X),
        Y - baseVal(slf, Key::position_last_Y)
    );

    // Reposition checks
    if (dY > 0.0 && dY > S) {
        Y = Y - dY + S;
    }
    else if (dY < 0.0 && - dY > N) {
        Y = Y - dY - N;
    }

    if (dX > 0.0 && dX > E) {
        X = X - dX + E;
    }
    else if (dX < 0.0 && - dX > W) {
        X = X - dX - W;
    }

    // Set clip values to max double for next frame's detection
    N = DBL_MAX;
    E = DBL_MAX;
    S = DBL_MAX;
    W = DBL_MAX;
}

} // namespace Nebulite::RulesetModule
