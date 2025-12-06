//------------------------------------------
// Includes

// Standard library
#include <cmath>

// Nebulite
#include "Nebulite.hpp"
#include "Core/RenderObject.hpp"
#include "Data/DocumentCache.hpp"
#include "Data/JSON.hpp"
#include "Interaction/Invoke.hpp"
#include "Interaction/Logic/Assignment.hpp"
#include "Interaction/Rules/Ruleset.hpp"

//------------------------------------------
namespace Nebulite::Interaction {

//------------------------------------------
// Constructor / Destructor

Invoke::Invoke() {
    // Linking an empty doc is needed for some functions
    emptyDoc = new Data::JSON("Empty JSON document for Invoke");

    // Initialize synchronization primitives
    threadState.stopFlag = false;
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        threadState.individualState[i].workReady = false;
        threadState.individualState[i].workFinished = false;

        // Start worker threads
        threadrunners[i] = std::thread([this, i] {
            while (!threadState.stopFlag) {
                // Wait for work to be ready
                std::unique_lock lock(broadcasted.entriesThisFrame[i].mutex);
                threadState.individualState[i].condition.wait(lock, [this, i] {
                    return threadState.individualState[i].workReady.load() || threadState.stopFlag.load();
                });
                // Process
                if (threadState.stopFlag)
                    break;
                processWork(broadcasted.entriesThisFrame[i].Container);
                threadState.individualState[i].workReady = false;
                threadState.individualState[i].workFinished = true;
            }
        });
    }
}

Invoke::~Invoke() {
    // Signal threads to stop and finish
    threadState.stopFlag = true;
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        threadState.individualState[i].condition.notify_one();
        if (threadrunners[i].joinable()) {
            threadrunners[i].join();
        }
    }
}

//------------------------------------------
// Checks

bool Invoke::checkRulesetLogicalCondition(Logic::ExpressionPool& expr, Core::RenderObject const* otherObj) {
    // Check if logical arg is as simple as just "1", meaning true
    if (expr.isAlwaysTrue())
        return true;

    double const result = expr.evalAsDouble(otherObj->getDoc());
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

//------------------------------------------
// Interactions

void Invoke::broadcast(std::shared_ptr<Rules::Ruleset> const& entry) {
    // Get index
    uint32_t const id_self = entry->id;
    uint32_t const threadIndex = id_self % THREADRUNNER_COUNT;

    // Insert into next frame's entries
    std::scoped_lock lock(broadcasted.entriesNextFrame[threadIndex].mutex);
    auto& [isActive, rulesets] = broadcasted.entriesNextFrame[threadIndex].Container[entry->topic][id_self];
    rulesets[entry->index].entry = entry;
    isActive = true;
}

void Invoke::listen(Core::RenderObject* obj, std::string const& topic, uint32_t const& listenerId) {
    for (auto& [container, mutex] : std::span(broadcasted.entriesThisFrame, THREADRUNNER_COUNT)) {
        // Lock to safely read from broadcasted.entriesThisFrame
        std::scoped_lock broadcastLock(mutex);

        // Check if any object has broadcasted on this topic
        auto topicIt = container.find(topic);
        if (topicIt == container.end()) {
            continue; // No entries for this topic in this thread
        }

        for (auto& [id_self, onTopicFromId] : topicIt->second) {
            // Skip if broadcaster and listener are the same object
            if (id_self == listenerId)
                continue;

            // Skip if inactive
            if (!onTopicFromId.active)
                continue;

            // For all rulesets under this broadcaster and topic
            for (auto& [entry, listeners] : std::ranges::views::values(onTopicFromId.rulesets)) {
                bool const pairStatus = checkRulesetLogicalCondition(entry->logicalArg, obj);
                listeners[listenerId] = BroadCastListenPair{entry, obj, pairStatus};
            }
        }
    }
}

//------------------------------------------
// Ruleset application

void Invoke::applyFunctionCalls(std::shared_ptr<Rules::Ruleset> const& ruleset, Core::RenderObject const* contextSelf, Core::RenderObject const* contextOther) const {
    // === Function Calls GLOBAL ===
    for (auto& entry : ruleset->functioncalls_global) {
        // replace vars
        std::string call = entry.eval(contextOther->getDoc());

        // attach to task queue
        std::scoped_lock lock(taskQueue.mutex);
        taskQueue.ptr->emplace_back(call);
    }

    // === Function Calls LOCAL: SELF ===
    for (auto& entry : ruleset->functioncalls_self) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(contextOther->getDoc());
        (void)contextSelf->parseStr(call);
    }

    // === Function Calls LOCAL: OTHER ===
    for (auto& entry : ruleset->functioncalls_other) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(contextOther->getDoc());
        (void)contextOther->parseStr(call);
    }
}

// TODO: if ruleset->staticFunction is not null, we need to always activate it
//       meaning we cannot check this here, as applyRuleset is only called
//       if the logical condition is true
//       But for static functions, that is determined inside the function itself
//       Another option would be to set logicalArg to always true for static functions?
//       -> Best compatibility!
void Invoke::applyRuleset(std::shared_ptr<Rules::Ruleset> const& ruleset, Core::RenderObject* contextOther) const {
    if (ruleset->staticFunction != nullptr) {
        // Static function, just call it
        Nebulite::Interaction::Rules::Context context{*ruleset->selfPtr, *contextOther, Nebulite::global()};
        ruleset->staticFunction(context);
        return;
    }

    // References
    Core::RenderObject const* contextSelf = ruleset->selfPtr;

    // Update self, other and global
    for (auto& assignment : ruleset->assignments) {
        assignment.apply(contextSelf->getDoc(), contextOther->getDoc());
    }

    // Apply function calls
    applyFunctionCalls(ruleset, contextSelf, contextOther);
}

void Invoke::applyRuleset(std::shared_ptr<Rules::Ruleset> const& ruleset) const {
    applyRuleset(ruleset, ruleset->selfPtr);
}

//------------------------------------------
// Update

void Invoke::update() {
    // Signal all worker threads to start processing
    for (auto& [condition, workReady, workFinished] : std::span(threadState.individualState, THREADRUNNER_COUNT)) {
        workReady = true;
        workFinished = false;
        condition.notify_one();
    }

    // Wait for all threads to finish processing
    for (auto& [condition, workReady, workFinished] : std::span(threadState.individualState, THREADRUNNER_COUNT)) {
        while (!workFinished.load()) {
            std::this_thread::yield(); // Yield to avoid busy waiting
        }
    }

    // Swap the containers, preparing for the next frame
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        // No workers active -> no mutex lock needed
        std::swap(broadcasted.entriesThisFrame[i].Container, broadcasted.entriesNextFrame[i].Container);
    }
}

//------------------------------------------
// Standalone Expression Evaluation

// String evaluation

std::string Invoke::evaluateStandaloneExpression(std::string const& input) const {
    Data::JSON* docSelf = this->emptyDoc; // no self context
    Data::JSON* docOther = this->emptyDoc; // no other context

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.eval(docOther);
}

std::string Invoke::evaluateStandaloneExpression(std::string const& input, Core::RenderObject const* selfAndOther) {
    // Expression is evaluated within a domain's context, use it as self and other
    Data::JSON* docSelf = selfAndOther->getDoc();
    Data::JSON* docOther = selfAndOther->getDoc();

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.eval(docOther);
}

// Double evaluation

double Invoke::evaluateStandaloneExpressionAsDouble(std::string const& input) const {
    Data::JSON* docSelf = this->emptyDoc; // no self context
    Data::JSON* docOther = this->emptyDoc; // no other context

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.evalAsDouble(docOther);
}

double Invoke::evaluateStandaloneExpressionAsDouble(std::string const& input, Core::RenderObject const* selfAndOther) {
    // Expression is evaluated within a domain's context, use it as self and other
    Data::JSON* docSelf = selfAndOther->getDoc();
    Data::JSON* docOther = selfAndOther->getDoc();

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.evalAsDouble(docOther);
}

// Boolean evaluation

bool Invoke::evaluateStandaloneExpressionAsBool(std::string const& input) const {
    Data::JSON* docSelf = this->emptyDoc; // no self context
    Data::JSON* docOther = this->emptyDoc; // no other context

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    double const result = expr.evalAsDouble(docOther);
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

bool Invoke::evaluateStandaloneExpressionAsBool(std::string const& input, Core::RenderObject const* selfAndOther) {
    // Expression is evaluated within a domain's context, use it as self and other
    Data::JSON* docSelf = selfAndOther->getDoc();
    Data::JSON* docOther = selfAndOther->getDoc();

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    double const result = expr.evalAsDouble(docOther);
    if (std::isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

//------------------------------------------
// Work processing

void Invoke::processWork(BroadCastListenContainer& container) {
    thread_local std::mt19937 cleanup_rng(std::random_device{}());
    thread_local std::uniform_int_distribution<int> cleanup_dist(0, 99); // uniform, avoids modulo bias
    for (auto& map_other : std::views::values(container)) {
        for (auto& [isActive, rulesets] : std::views::values(map_other)) {
            if (!isActive)
                continue;
            for (auto& [entry, listeners] : std::ranges::views::values(rulesets)) {
                // Process active listeners (single pass, no erases here)
                for (auto & it : listeners) {
                    auto &pair = it.second;
                    if (pair.active) {
                        applyRuleset(pair.entry, pair.contextOther);
                        pair.active = false;
                    }
                }
                // Probabilistic cleanup performed once per ruleset
                if (cleanup_dist(cleanup_rng) == 0) {
                    for (auto it = listeners.begin(); it != listeners.end();) {
                        if (!it->second.active) {
                            auto itToErase = it++;
                            listeners.erase(itToErase); // erase returns void in Abseil
                        } else {
                            ++it;
                        }
                    }
                }
            }
            // Reset activity flag, must be activated on broadcast
            isActive = false;
        }
    }
}

} // namespace Nebulite::Interaction