#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"

namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Base Class Virtual Methods

bool Ruleset::evaluateCondition(Interaction::Execution::DomainBase const* /*other*/) {
    return false;
}

bool Ruleset::evaluateCondition() {
    return evaluateCondition(selfPtr);
}

void Ruleset::apply(Interaction::Execution::DomainBase* /*contextOther*/) {
    // default no-op
}

void Ruleset::apply() {
    apply(selfPtr);
}

//------------------------------------------
// Derived Class Methods: JsonRuleset

bool JsonRuleset::evaluateCondition(Interaction::Execution::DomainBase const* otherObj) {
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

void JsonRuleset::apply(Interaction::Execution::DomainBase* contextOther) {
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
        // TODO: needs its own lock!!
        //       Idea: don't expose task queue publicly, but have a method to add tasks with internal locking
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