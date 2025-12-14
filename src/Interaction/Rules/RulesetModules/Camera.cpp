#include "Interaction/Rules/RulesetModules/Camera.hpp"
#include "Interaction/Rules/StaticRulesetMap.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

// TODO: Additional functions for alignment to object edges

void Camera::alignCenter(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Center);
}

void Camera::alignTop(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Top);
}

void Camera::alignBottom(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Bottom);
}

void Camera::alignLeft(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Left);
}

void Camera::alignRight(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf, Align::Center), Align::Right);
}


} // namespace Nebulite::Interaction::Rules::RulesetModules
