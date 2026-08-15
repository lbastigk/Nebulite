//------------------------------------------
// Includes

// Standard library
#include <cstddef>
#include <memory>
#include <string>

// Nebulite
#include "Nebulite/Interaction/Context.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Interaction/GlobalValue.hpp"
#include "Nebulite/Interaction/Rules/Listener.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Utility/Promise.hpp"

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

void Ruleset::applyContext(Context& /*context*/, ContextScope& /*contextScope*/, GlobalValueCopy const& /*globalValueCopy*/){
    // default no-op
}

void Ruleset::applyListener(std::shared_ptr<Listener> const& /*listener*/, Execution::Domain& /*global*/, GlobalValueCopy const& /*globalValueCopy*/) {
    // default no-op
}

void Ruleset::applyDomain(Execution::Domain& /*global*/, GlobalValueCopy const& /*globalValueCopy*/) {
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

void StaticRuleset::applyContext(Context& context, ContextScope& /*contextScope*/, GlobalValueCopy const& globalValueCopy) {
    auto* slfFromProvidedContext = baseListFunction(context.self);
    auto* otrFromProvidedContext = baseListFunction(context.other);
    staticFunction(context, slfFromProvidedContext, otrFromProvidedContext, globalValueCopy);
}

void StaticRuleset::applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global, GlobalValueCopy const& globalValueCopy) {
    Context const context{self, listener->domain, global};
    staticFunction(context, slf, listener->otr, globalValueCopy);
}

void StaticRuleset::applyDomain(Execution::Domain& global, GlobalValueCopy const& globalValueCopy) {
    Context const context{self, self, global};
    staticFunction(context, slf, slf, globalValueCopy);
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

    // Promise was fulfilled during ruleset parsing
    ContextScope const contextScope{self.domainScope, other.domainScope, global.domainScope};
    return logicalArg->evalAsBool(contextScope, Utility::Promise<&Logic::Expression::isReturnableAsBool>{});
}

void JsonRuleset::applyContext(Context& context, ContextScope& contextScope, GlobalValueCopy const& /*globalValueCopy*/) {
    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(contextScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncallsGlobal) {
        sendTask(context.global, entry.eval(contextScope));
    }
    for (auto& entry : functioncallsSelf) {
        sendTask(context.self, entry.eval(contextScope));
    }
    for (auto& entry : functioncallsOther) {
        sendTask(context.other, entry.eval(contextScope));
    }
}

void JsonRuleset::applyListener(std::shared_ptr<Listener> const& listener, Execution::Domain& global, GlobalValueCopy const& globalValueCopy) {
    Context ctx{self, listener->domain, global};
    ContextScope contextScope{self.domainScope, listener->domain.domainScope, global.domainScope};
    applyContext(ctx, contextScope, globalValueCopy);
}

void JsonRuleset::applyDomain(Execution::Domain& global, GlobalValueCopy const& /*globalValueCopy*/) {
    Context const ctx{self, self, global};
    ContextScope const ctxScope{self.domainScope, self.domainScope, global.domainScope};

    // 1.) Assignments
    for (auto& assignment : assignments) {
        assignment.apply(ctxScope);
    }

    // 2.) Function calls
    for (auto& entry : functioncallsGlobal) {
        sendTask(ctx.global, entry.eval(ctxScope));
    }
    for (auto& entry : functioncallsSelf) {
        sendTask(ctx.self, entry.eval(ctxScope));
    }
    for (auto& entry : functioncallsOther) {
        sendTask(ctx.other, entry.eval(ctxScope));
    }
}

} // namespace Nebulite::Interaction::Rules
