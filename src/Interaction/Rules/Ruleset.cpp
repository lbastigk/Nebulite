//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <memory>
#include <string>

// Nebulite
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/Rules/Listener.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Base Class Virtual Methods

std::size_t Ruleset::getId() const { return self.getId(); }

std::size_t Ruleset::getIdHashed() const { return self.getIdHashed(); }

bool Ruleset::evaluateConditionGlobally(Execution::Domain& /*self*/, Execution::Domain& /*global*/) {
    return false;
}

bool Ruleset::evaluateConditionLocally(Execution::Domain& global) {
    return evaluateConditionGlobally(self, global);
}

void Ruleset::applyContext(Context& /*context*/, ContextScope& /*contextScope*/){
    // default no-op
}

void Ruleset::applyListener(std::shared_ptr<Listener> const& /*listener*/, Execution::Domain& /*global*/) {
    // default no-op
}

void Ruleset::applyDomain(Execution::Domain& /*global*/) {
    // default no-op
}

//------------------------------------------
// Derived Class Methods: StaticRuleset

bool StaticRuleset::evaluateConditionGlobally(Execution::Domain& /*other*/, Execution::Domain& /*global*/) {
    return true;
}

bool StaticRuleset::evaluateConditionLocally(Execution::Domain& /*global*/) {
    return true;
}

void StaticRuleset::applyContext(Context& context, ContextScope& /*contextScope*/){
    auto* slfFromProvidedContext = baseListFunction(context.self);
    auto* otrFromProvidedContext = baseListFunction(context.other);
    staticFunction(context, slfFromProvidedContext, otrFromProvidedContext);
}

void StaticRuleset::applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global) {
    Context const context{self, listener->domain, global};
    staticFunction(context, slf, listener->otr);
}

void StaticRuleset::applyDomain(Execution::Domain& global) {
    Context const context{self, self, global};
    staticFunction(context, slf, slf);
}

//------------------------------------------
// Task handling

namespace {
void sendTask(Execution::Domain& domain, std::string const& task) {
    domain.tasks.addTask(task);
}
} // namespace

//------------------------------------------
// Derived Class Methods: JsonRuleset

bool JsonRuleset::evaluateConditionGlobally(Execution::Domain& other, Execution::Domain& global) {
    // Check if logical arg is as simple as just "1", meaning true
    if (logicalArg->isAlwaysTrue()) {
        return true;
    }

    ContextScope const contextScope{self.domainScope, other.domainScope, global.domainScope};
    return logicalArg->evalAsBool(contextScope);
}

void JsonRuleset::applyContext(Context& context, ContextScope& contextScope){
    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(contextScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        sendTask(context.global, entry.eval(contextScope));
    }
    for (auto& entry : functioncalls_self) {
        sendTask(context.self, entry.eval(contextScope));
    }
    for (auto& entry : functioncalls_other) {
        sendTask(context.other, entry.eval(contextScope));
    }
}

void JsonRuleset::applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global) {
    Context ctx{self, listener->domain, global};
    ContextScope contextScope{self.domainScope, listener->domain.domainScope, global.domainScope};
    applyContext(ctx, contextScope);
}

void JsonRuleset::applyDomain(Execution::Domain& global) {
    Context const ctx{self, self, global};
    ContextScope const ctxScope{self.domainScope, self.domainScope, global.domainScope};

    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(ctxScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncalls_global) {
        sendTask(ctx.global, entry.eval(ctxScope));
    }
    for (auto& entry : functioncalls_self) {
        sendTask(ctx.self, entry.eval(ctxScope));
    }
    for (auto& entry : functioncalls_other) {
        sendTask(ctx.other, entry.eval(ctxScope));
    }
}

} // namespace Nebulite::Interaction::Rules
