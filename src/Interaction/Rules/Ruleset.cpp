//------------------------------------------
// Includes

// Standard library
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>

// Nebulite
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"
#include "Interaction/Rules/Listener.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Interaction::Rules {

//------------------------------------------
// Base Class Virtual Methods

size_t const& Ruleset::getId() const { return self.getId(); }

size_t const& Ruleset::getIdHashed() const { return self.getIdHashed(); }

bool Ruleset::evaluateConditionGlobally(Execution::Domain& /*self*/, Execution::Domain& /*global*/) {
    return false;
}

bool Ruleset::evaluateConditionLocally(Execution::Domain& global) {
    return evaluateConditionGlobally(self, global);
}

void Ruleset::apply(Context& /*context*/, ContextScope& /*contextScope*/){
    // default no-op
}

void Ruleset::apply(std::shared_ptr<Listener> const& /*listener*/, Execution::Domain& /*global*/) {
    // default no-op
}

void Ruleset::apply(Execution::Domain& /*global*/) {
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

void StaticRuleset::apply(Context& context, ContextScope& /*contextScope*/){
    auto* slfFromProvidedContext = baseListFunction(context.self);
    auto* otrFromProvidedContext = baseListFunction(context.other);
    staticFunction(context, slfFromProvidedContext, otrFromProvidedContext);
}

void StaticRuleset::apply(std::shared_ptr<Listener> const& listener, Execution::Domain& global) {
    Context const context{self, listener->domain, global};
    staticFunction(context, slf, listener->otr);
}

void StaticRuleset::apply(Execution::Domain& global) {
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
    if (logicalArg->isAlwaysTrue())
        return true;

    ContextScope const contextScope{self.domainScope, other.domainScope, global.domainScope};
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
        sendTask(context.global, entry.eval(contextScope));
    }
    for (auto& entry : functioncalls_self) {
        sendTask(context.self, entry.eval(contextScope));
    }
    for (auto& entry : functioncalls_other) {
        sendTask(context.other, entry.eval(contextScope));
    }
}

void JsonRuleset::apply(std::shared_ptr<Listener> const& listener, Execution::Domain& global) {
    Context ctx{self, listener->domain, global};
    ContextScope contextScope{self.domainScope, listener->domain.domainScope, global.domainScope};
    apply(ctx, contextScope);
}

void JsonRuleset::apply(Execution::Domain& global) {
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
