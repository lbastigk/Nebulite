#include "Nebulite.hpp"
#include "Interaction/Rules/RulesetModules/Camera.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

Camera::Camera() : RulesetModule(moduleName) {
    // Bind Camera-related static rulesets here
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignCenter, alignCenterName, alignCenterDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignTop, alignTopName, alignTopDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignBottom, alignBottomName, alignBottomDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignLeft, alignLeftName, alignLeftDesc);
    BIND_STATIC_ASSERT(RulesetType::Local, &Camera::alignRight, alignRightName, alignRightDesc);

    // References
    globalVal.camPosX = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.positionX);
    globalVal.camPosY = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.positionY);
    globalVal.dispResX = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.dispResX);
    globalVal.dispResY = Nebulite::global().getDoc()->getStableDoublePointer(Nebulite::Constants::keyName.renderer.dispResY);
}

// TODO: Additional functions for alignment to object edges

void Camera::alignCenter(ContextBase const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Center);
}

void Camera::alignTop(ContextBase const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Top);
}

void Camera::alignBottom(ContextBase const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Bottom);
}

void Camera::alignLeft(ContextBase const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Left);
}

void Camera::alignRight(ContextBase const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Right);
}


} // namespace Nebulite::Interaction::Rules::RulesetModules
