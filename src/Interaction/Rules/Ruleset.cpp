#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"

namespace Nebulite::Interaction::Rules {
bool Ruleset::evaluateCondition(Core::RenderObject const* otherObj) {
    // Check if logical arg is as simple as just "1", meaning true
    if (logicalArg.isAlwaysTrue())
        return true;

    double const result = logicalArg.evalAsDouble(otherObj->getDoc());
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

void Ruleset::apply(Core::RenderObject* contextOther) {
    if (staticFunction != nullptr) {
        // Static function, just call it
        Nebulite::Interaction::Rules::Context context{*selfPtr, *contextOther, Nebulite::global()};
        staticFunction(context);
        return;
    }

    auto selfDoc = selfPtr->getDoc();
    auto otherDoc = contextOther->getDoc();

    // 1.) Assignments
    // TODO: getDoc before the loop!
    for (auto& assignment : assignments) {
        assignment.apply(selfDoc, otherDoc);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(otherDoc);

        // attach to task queue
        // TODO: needs its own lock!
        //std::scoped_lock lock(taskQueue.mutex);
        //Nebulite::global().getTaskQueue()->emplace_back(call);
    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(otherDoc);
        (void)selfPtr->parseStr(call);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(otherDoc);
        (void)contextOther->parseStr(call);
    }
}

} // namespace Nebulite::Interaction::Rules