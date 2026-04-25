#include "Nebulite.hpp"
#include "Module/Ruleset/Camera.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

namespace Nebulite::Module::Ruleset {

Camera::Camera() : RulesetModule(moduleName) {
    auto const baseListFunc = generateBaseListFunction(baseKeys);

    // Bind Camera-related static rulesets here
    bind<alignCenterName>(RulesetType::Local, &Camera::alignCenter, alignCenterDesc, baseListFunc);
    bind<alignTopName>(RulesetType::Local, &Camera::alignTop, alignTopDesc, baseListFunc);
    bind<alignBottomName>(RulesetType::Local, &Camera::alignBottom, alignBottomDesc, baseListFunc);
    bind<alignLeftName>(RulesetType::Local, &Camera::alignLeft, alignLeftDesc, baseListFunc);
    bind<alignRightName>(RulesetType::Local, &Camera::alignRight, alignRightDesc, baseListFunc);

    // References
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.camPosX = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionX);
    globalVal.camPosY = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionY);
    globalVal.dispResX = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResX);
    globalVal.dispResY = Global::shareScope(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResY);
}

// TODO: Add another namespace for camera following rulesets using a PT1 controller for smooth movement

void Camera::alignCenter(Interaction::Context const& /*context*/, double**& slf, double**&) const {
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Center);
}

void Camera::alignTop(Interaction::Context const& /*context*/, double**& slf, double**&) const {
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Top);
}

void Camera::alignBottom(Interaction::Context const& /*context*/, double**& slf, double**&) const {
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Bottom);
}

void Camera::alignLeft(Interaction::Context const& /*context*/, double**& slf, double**&) const {
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Left);
}

void Camera::alignRight(Interaction::Context const& /*context*/, double**& slf, double**&) const {
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Right);
}


} // namespace Nebulite::Module::Ruleset
