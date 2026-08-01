//------------------------------------------
// Includes

// Standard Library
#include <cassert>

// Nebulite
#include "Nebulite/Constants/KeyNames.hpp"
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"
#include "Nebulite/Module/Ruleset/Camera.hpp"
#include "Nebulite/Nebulite.hpp"

//------------------------------------------
namespace Nebulite::Module::Ruleset {

Camera::Camera() : RulesetModule(moduleName, this) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Bind Camera-related static rulesets here
    bind<alignCenterName>(&Camera::alignCenter, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, alignCenterDesc);
    bind<alignTopName>(&Camera::alignTop, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, alignTopDesc);
    bind<alignBottomName>(&Camera::alignBottom, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, alignBottomDesc);
    bind<alignLeftName>(&Camera::alignLeft, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, alignLeftDesc);
    bind<alignRightName>(&Camera::alignRight, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, alignRightDesc);

    // References
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.camPosX = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionX);
    globalVal.camPosY = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionY);
    globalVal.dispResX = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResXLogical);
    globalVal.dispResY = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResYLogical);
}

// TODO: Add another namespace for camera following rulesets using a PT1 controller for smooth movement

void Camera::alignCenter([[maybe_unused]] Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::center), Align::center);
}

void Camera::alignTop([[maybe_unused]] Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::center), Align::top);
}

void Camera::alignBottom([[maybe_unused]] Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::center), Align::bottom);
}

void Camera::alignLeft([[maybe_unused]] Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::center), Align::left);
}

void Camera::alignRight([[maybe_unused]] Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::center), Align::right);
}

void Camera::setCameraPosition(Position const& pos, Align const& align) const {
    switch (align) {
        case Align::top:
            *globalVal.camPosY = pos.y ;
            break;
        case Align::bottom:
            *globalVal.camPosY = pos.y - *globalVal.dispResY;
            break;
        case Align::left:
            *globalVal.camPosX = pos.x;
            break;
        case Align::right:
            *globalVal.camPosX = pos.x - *globalVal.dispResX;
            break;
        case Align::center:
        default: // Fallback to center
            *globalVal.camPosX = pos.x - *globalVal.dispResX / 2.0;
            *globalVal.camPosY = pos.y - *globalVal.dispResY / 2.0;
            break;
    }
}

Camera::Position Camera::getAdjustedObjectPosition(double** baseValues, Align const& align) {
    // Adjust based on object size
    Position pos;
    switch (align) {
        // Not used at the moment:
        // NOLINTBEGIN
        case Align::top:
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY);
            break;
        case Align::bottom:
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
            pos.y = baseVal(baseValues, Key::posY);
            break;
        case Align::left:
            pos.x = baseVal(baseValues, Key::posX);
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
            break;
        case Align::right:
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX);
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
            break;
        // NOLINTEND
        case Align::center:
        default: // Fallback to center
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
            break;
    }
    return pos;
}

} // namespace Nebulite::Module::Ruleset
