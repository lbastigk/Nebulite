#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"

namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Base Class Virtual Methods

bool Ruleset::evaluateCondition(Execution::Domain const& /*other*/) {
    return false;
}

bool Ruleset::evaluateCondition() {
    return evaluateCondition(self);
}

void Ruleset::apply(Execution::Domain& /*contextOther*/) {
    // default no-op
}

void Ruleset::apply() {
    apply(self);
}

//------------------------------------------
// Derived Class Methods: StaticRuleset

void StaticRuleset::apply(Execution::Domain& contextOther) {
    Context const contextBase{self, contextOther, Global::instance()};
    staticFunction(contextBase);
}

//------------------------------------------
// Derived Class Methods: JsonRuleset

bool JsonRuleset::evaluateCondition(Execution::Domain const& other) {
    // Check if logical arg is as simple as just "1", meaning true
    if (logicalArg->isAlwaysTrue())
        return true;

    double const result = logicalArg->evalAsDouble(other.domainScopeBase());
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

void JsonRuleset::apply(Execution::Domain& contextOther) {
    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(self.domainScope, contextOther.domainScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(contextOther.domainScopeBase());

        // attach to task queue
        Global::instance().getTaskQueue(Core::GlobalSpace::StandardTasks::internal)->pushBack(call);

    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = entry.eval(contextOther.domainScopeBase());
        (void)self.parseStr(call);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = entry.eval(contextOther.domainScopeBase());
        (void)contextOther.parseStr(call);
    }
}

} // namespace Nebulite::Interaction::Rules
