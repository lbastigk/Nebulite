#include "Nebulite.hpp"
#include "Interaction/Rules/Ruleset.hpp"

namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Base Class Virtual Methods

size_t const& Ruleset::getId() const { return self.getId(); }

size_t const& Ruleset::getIdHashed() const { return self.getIdHashed(); }

bool Ruleset::evaluateCondition(Execution::Domain& /*other*/) {
    return false;
}

bool Ruleset::evaluateCondition() {
    return evaluateCondition(self);
}

void Ruleset::apply(std::shared_ptr<Listener> const& /*contextOther*/) {
    // default no-op
}

void Ruleset::apply() {
    // default no-op
}

//------------------------------------------
// Derived Class Methods: StaticRuleset

void StaticRuleset::apply(std::shared_ptr<Listener> const& contextOther) {
    Context const context{self, contextOther->domain, Global::instance()};
    staticFunction(context, slf, contextOther->otr);
}

void StaticRuleset::apply() {
    Context const context{self, self, Global::instance()};
    staticFunction(context, slf, slf);
}

//------------------------------------------
// Derived Class Methods: JsonRuleset

bool JsonRuleset::evaluateCondition(Execution::Domain& other) {
    // Check if logical arg is as simple as just "1", meaning true
    if (logicalArg->isAlwaysTrue())
        return true;

    Context const context{self, other, Global::instance()};
    double const result = logicalArg->evalAsDouble(context);
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

void JsonRuleset::apply(std::shared_ptr<Listener> const& contextOther) {
    ContextScopeBase const context{self.domainScope, contextOther->domain.domainScope, Global::instance().domainScope};

    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(context);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(context);

        // attach to task queue
        Global::instance().getTaskQueue(Core::GlobalSpace::StandardTasks::internal)->pushBack(call);

    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = entry.eval(context);
        (void)self.parseStr(call);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = entry.eval(context);
        (void)contextOther->domain.parseStr(call);
    }
}

void JsonRuleset::apply() {
    ContextScopeBase const context{self.domainScope, self.domainScope, Global::instance().domainScope};

    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(context);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(context);

        // attach to task queue
        Global::instance().getTaskQueue(Core::GlobalSpace::StandardTasks::internal)->pushBack(call);

    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = entry.eval(context);
        (void)self.parseStr(call);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = entry.eval(context);
        (void)self.parseStr(call);
    }
}

} // namespace Nebulite::Interaction::Rules
