//------------------------------------------
// Includes

// Nebulite
#include "Interaction/Rules/Listener.hpp"
#include "Interaction/Rules/Ruleset.hpp"
#include "Nebulite.hpp"

//------------------------------------------
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

void Ruleset::apply(Context& /*context*/, ContextScope& /*contextScope*/){
    // default no-op
}

void Ruleset::apply(std::shared_ptr<Listener> const& /*contextOther*/) {
    // default no-op
}

void Ruleset::apply() {
    // default no-op
}

//------------------------------------------
// Derived Class Methods: StaticRuleset

void StaticRuleset::apply(Context& context, ContextScope& /*contextScope*/){
    auto slfFromProvidedContext = baseListFunction(context.self);
    auto otrFromProvidedContext = baseListFunction(context.other);
    staticFunction(context, slfFromProvidedContext, otrFromProvidedContext);
}

// TODO: don't assume we can use Global::instance(), use a provided global domain, once taskqueue appending is a part of any domain

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

    ContextScope const contextScope{self.domainScope, other.domainScope, Global::instance().domainScope};
    double const result = logicalArg->evalAsDouble(contextScope);
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

void JsonRuleset::apply(Context& context, ContextScope& contextScope){

    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(contextScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(contextScope);

        // TODO: Add taskQueue to any domain, then use context.global.getTaskQueue
        // attach to task queue
        Global::instance().getTaskQueue(Core::GlobalSpace::StandardTasks::internal)->pushBack(call);
    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = entry.eval(contextScope);
        (void)context.self.parseStr(call, context, contextScope);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = entry.eval(contextScope);
        (void)context.other.parseStr(call, context, contextScope);
    }
}

void JsonRuleset::apply(std::shared_ptr<Listener> const& contextOther) {
    Context ctx{self, contextOther->domain, Global::instance()};
    ContextScope contextScope{self.domainScope, contextOther->domain.domainScope, Global::instance().domainScope};
    apply(ctx, contextScope);
}

void JsonRuleset::apply() {
    Context ctx{self, self, Global::instance()};
    ContextScope ctxScope{self.domainScope, self.domainScope, Global::instance().domainScope};

    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(ctxScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        // replace vars
        std::string call = entry.eval(ctxScope);

        // attach to task queue
        Global::instance().getTaskQueue(Core::GlobalSpace::StandardTasks::internal)->pushBack(call);
    }
    for (auto& entry : functioncalls_self) {
        // replace vars
        std::string const call = entry.eval(ctxScope);
        (void)self.parseStr(call, ctx, ctxScope);
    }
    for (auto& entry : functioncalls_other) {
        // replace vars
        std::string const call = entry.eval(ctxScope);
        (void)self.parseStr(call, ctx, ctxScope);
    }
}

} // namespace Nebulite::Interaction::Rules
