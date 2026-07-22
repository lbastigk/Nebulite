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

Camera::Camera() : RulesetModule(moduleName) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Bind Camera-related static rulesets here
    bind<alignCenterName>(&Camera::alignCenter, baseListFunc, Interaction::Rules::StaticRuleset::Type::Local, alignCenterDesc);
    bind<alignTopName>(&Camera::alignTop, baseListFunc, Interaction::Rules::StaticRuleset::Type::Local, alignTopDesc);
    bind<alignBottomName>(&Camera::alignBottom, baseListFunc, Interaction::Rules::StaticRuleset::Type::Local, alignBottomDesc);
    bind<alignLeftName>(&Camera::alignLeft, baseListFunc, Interaction::Rules::StaticRuleset::Type::Local, alignLeftDesc);
    bind<alignRightName>(&Camera::alignRight, baseListFunc, Interaction::Rules::StaticRuleset::Type::Local, alignRightDesc);

    // References
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.camPosX = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionX);
    globalVal.camPosY = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionY);
    globalVal.dispResX = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResXLogical);
    globalVal.dispResY = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResYLogical);
}

// TODO: Add another namespace for camera following rulesets using a PT1 controller for smooth movement

void Camera::alignCenter(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Center);
}

void Camera::alignTop(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Top);
}

void Camera::alignBottom(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Bottom);
}

void Camera::alignLeft(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Left);
}

void Camera::alignRight(Interaction::Context const& context, double** slf, double** /*otr*/) const {
    assert(isGlobalContextCorrect(context));
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Right);
}

void Camera::setCameraPosition(const Position& pos, Align const& align) const {
    switch (align) {
        case Align::Top:
            *globalVal.camPosY = pos.y ;
            break;
        case Align::Bottom:
            *globalVal.camPosY = pos.y - *globalVal.dispResY;
            break;
        case Align::Left:
            *globalVal.camPosX = pos.x;
            break;
        case Align::Right:
            *globalVal.camPosX = pos.x - *globalVal.dispResX;
            break;
        case Align::Center:
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
        case Align::Top:
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY);
            break;
        case Align::Bottom:
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
            pos.y = baseVal(baseValues, Key::posY);
            break;
        case Align::Left:
            pos.x = baseVal(baseValues, Key::posX);
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
            break;
        case Align::Right:
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX);
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
            break;
        // NOLINTEND
        case Align::Center:
        default: // Fallback to center
            pos.x = baseVal(baseValues, Key::posX) + baseVal(baseValues, Key::spriteSizeX) / 2.0;
            pos.y = baseVal(baseValues, Key::posY) + baseVal(baseValues, Key::spriteSizeY) / 2.0;
            break;
    }
    return pos;
}

} // namespace Nebulite::Module::Ruleset
