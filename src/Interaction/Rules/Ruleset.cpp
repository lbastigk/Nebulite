#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"

namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Base Class Virtual Methods

bool Ruleset::evaluateCondition(Execution::DomainBase const* /*other*/) {
    return false;
}

bool Ruleset::evaluateCondition() {
    return evaluateCondition(selfPtr);
}

void Ruleset::apply(Execution::DomainBase* /*contextOther*/) {
    // default no-op
}

void Ruleset::apply() {
    apply(selfPtr);
}

//------------------------------------------
// Derived Class Methods: StaticRuleset

void StaticRuleset::apply(Execution::DomainBase* contextOther) {
    ContextBase const contextBase{*selfPtr, *contextOther, Global::instance()};
    staticFunction(contextBase);
}

//------------------------------------------
// Derived Class Methods: JsonRuleset

bool JsonRuleset::evaluateCondition(Execution::DomainBase const* other) {
    // Check if logical arg is as simple as just "1", meaning true
    if (logicalArg->isAlwaysTrue())
        return true;

    double const result = logicalArg->evalAsDouble(other->domainScope);
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

void JsonRuleset::apply(Execution::DomainBase* contextOther) {
    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(selfPtr->domainScope, contextOther->domainScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(contextOther->domainScope);

        // attach to task queue
        Global::instance().getTaskQueue(Core::GlobalSpace::StandardTasks::internal)->pushBack(call);

    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(contextOther->domainScope);
        (void)selfPtr->parseStr(call);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(contextOther->domainScope);
        (void)contextOther->parseStr(call);
    }
}

} // namespace Nebulite::Interaction::Rules
