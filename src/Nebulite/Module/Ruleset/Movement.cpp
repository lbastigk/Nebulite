//------------------------------------------
// Includes

// Standard Library
#include <algorithm>
#include <limits>
#include <utility>

// Nebulite
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Rules/StaticRulesetMap.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"
#include "Nebulite/Module/Ruleset/Movement.hpp"

//------------------------------------------
namespace Nebulite::Module::Ruleset {

Movement::Movement() : RulesetModule(moduleName, this) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Global rulesets
    bindStaticFunction<detectClippingName, Movement>(&Movement::detectClipping, baseListFunc, Interaction::Rules::StaticRuleset::Type::global, detectClippingDesc);

    // Local rulesets
    bindStaticFunction<processClippingName, Movement>(&Movement::processClipping, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, processClippingDesc);
}

// NOLINTNEXTLINE
void Movement::detectClipping(Interaction::Context const& context, double** slf, double** otr, Interaction::GlobalValueCopy const& /*global*/) {
    // TODO: Add missing clipping detections
    // Assume context part is circle if radius is set (> 0)
    if (auto const& radius = Radius(slf, otr); radius.slf > 0.0 && radius.otr > 0.0) {
        context.self.capture.error.println("Circle-circle clipping detection not implemented yet");
    }
    else if (radius.slf > 0.0) {
        context.self.capture.error.println("Circle-rectangle clipping detection not implemented yet");
    }
    else if (radius.otr > 0.0) {
        context.self.capture.error.println("Rectangle-circle clipping detection not implemented yet");
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
        double const& m1 = baseVal(slf, Key::physicsMass);
        double const& m2 = baseVal(otr, Key::physicsMass);

        if (bool const baseCondition = m1 > 0.0 && m2 > 0.0; baseCondition) {
            // Determine if they align along an axis
            bool const axisAlignX = std::max(p1X, p2X) < std::min(p1X + size1X, p2X + size2X);
            bool const axisAlignY = std::max(p1Y, p2Y) < std::min(p1Y + size1Y, p2Y + size2Y);

            bool const isNorth = axisAlignX && p1Y + size1Y <= p2Y;
            bool const isEast  = axisAlignY && p1X >= p2X + size2X;
            bool const isSouth = axisAlignX && p1Y >= p2Y + size2Y;
            bool const isWest  = axisAlignY && p1X + size1X <= p2X;

            if (isNorth) {
                double const dist = p2Y - p1Y - size1Y;
                auto lock = context.other.lockDocument();
                double& closestCurrent = baseVal(otr, Key::clipClosestN);
                closestCurrent = std::min(closestCurrent, dist);
            }
            if (isEast) {
                double const dist = p1X - p2X - size2X;
                auto lock = context.other.lockDocument();
                double& closestCurrent = baseVal(otr, Key::clipClosestE);
                closestCurrent = std::min(closestCurrent, dist);
            }
            if (isSouth) {
                double const dist = p1Y - p2Y - size2Y;
                auto lock = context.other.lockDocument();
                double& closestCurrent = baseVal(otr, Key::clipClosestS);
                closestCurrent = std::min(closestCurrent, dist);
            }
            if (isWest) {
                double const dist = p2X - p1X - size1X;
                auto lock = context.other.lockDocument();
                double& closestCurrent = baseVal(otr, Key::clipClosestW);
                closestCurrent = std::min(closestCurrent, dist);
            }
        }
    }
}

// NOLINTNEXTLINE
void Movement::processClipping(Interaction::Context const& /*context*/, double** slf, double** /*otr*/, Interaction::GlobalValueCopy const& /*global*/) {
    // Nearest corners
    double& directionN = baseVal(slf, Key::clipClosestN);
    double& directionE = baseVal(slf, Key::clipClosestE);
    double& directionS = baseVal(slf, Key::clipClosestS);
    double& directionW = baseVal(slf, Key::clipClosestW);

    double& posX = baseVal(slf, Key::posX);
    double& posY = baseVal(slf, Key::posY);

    auto const [dX, dY] = std::make_pair(
        posX - baseVal(slf, Key::lastPositionX),
        posY - baseVal(slf, Key::lastPositionY)
    );

    // Reposition checks
    if (dY > 0.0 && dY > directionS) {
        posY = posY - dY + directionS;
    }
    else if (dY < 0.0 && - dY > directionN) {
        posY = posY - dY - directionN;
    }

    if (dX > 0.0 && dX > directionE) {
        posX = posX - dX + directionE;
    }
    else if (dX < 0.0 && - dX > directionW) {
        posX = posX - dX - directionW;
    }

    // Set clip values to max double for next frame's detection
    directionN = std::numeric_limits<double>::max();
    directionE = std::numeric_limits<double>::max();
    directionS = std::numeric_limits<double>::max();
    directionW = std::numeric_limits<double>::max();
}

} // namespace Nebulite::Module::Ruleset
