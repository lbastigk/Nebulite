#include "Interaction/Rules/RulesetModules/Camera.hpp"
#include "Interaction/Rules/StaticRulesets.hpp"

namespace Nebulite::Interaction::Rules::RulesetModules {

void Camera::alignCenter(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf), Align::Center);
}

void Camera::alignTop(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf), Align::Top);
}

void Camera::alignBottom(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf), Align::Bottom);
}

void Camera::alignLeft(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf), Align::Left);
}

void Camera::alignRight(Context const& context) {
    double** slf = getBaseList(context.self);
    setCameraPosition(getAdjustedObjectPosition(slf), Align::Right);
}


} // namespace Nebulite::Interaction::Rules::RulesetModules
