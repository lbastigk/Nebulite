#include "Nebulite.hpp"
#include "RulesetModule/Camera.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

namespace Nebulite::RulesetModule {

Camera::Camera() : RulesetModule(moduleName) {
    // Bind Camera-related static rulesets here
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignCenter, alignCenterName, alignCenterDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignTop, alignTopName, alignTopDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignBottom, alignBottomName, alignBottomDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignLeft, alignLeftName, alignLeftDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignRight, alignRightName, alignRightDesc);

    // References
    auto const token = getRulesetModuleAccessToken(*this);
    globalVal.camPosX = Global::shareScopeBase(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionX);
    globalVal.camPosY = Global::shareScopeBase(token).getStableDoublePointer(Constants::KeyNames::Renderer::positionY);
    globalVal.dispResX = Global::shareScopeBase(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResX);
    globalVal.dispResY = Global::shareScopeBase(token).getStableDoublePointer(Constants::KeyNames::Renderer::dispResY);
}

// TODO: Add another namespace for camera following rulesets using a PT1 controller for smooth movement

void Camera::alignCenter(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Center);
}

void Camera::alignTop(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Top);
}

void Camera::alignBottom(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Bottom);
}

void Camera::alignLeft(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Left);
}

void Camera::alignRight(Interaction::Context const& context, double**& slf, double**&) const {
    ensureBaseList(context.self, baseKeys, slf);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Right);
}


} // namespace Nebulite::RulesetModule
