#include "Interaction/Invoke.hpp"
#include "Core/RenderObject.hpp"

#include "Nebulite.hpp"

//------------------------------------------
// Constructor / Destructor

Nebulite::Interaction::Invoke::Invoke() {
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

                if (threadState.stopFlag)
                    break;

                // Process work
                for (auto& map_other : std::views::values(broadcasted.entriesThisFrame[i].Container)) {
                    for (auto& [isActive, rulesets] : std::views::values(map_other)) {
                        if (!isActive)
                            continue; // Skip if self did not broadcast this frame
                        for (auto& [entry, listeners] : std::ranges::views::values(rulesets)) {
                            for (auto it = listeners.begin(); it != listeners.end();) {
                                if (it->second.active) {
                                    applyRulesets(it->second.entry, it->second.Obj_other);
                                    it->second.active = false; // Reset for next frame
                                    ++it;
                                } else {
                                    // Probabilistic cleanup of inactive listeners
                                    // Use thread_local RNG for better performance
                                    // Since inactive listeners are rare, this should be fine
                                    // Sort of self-regulating, as more inactive listeners lead to more frequent cleanups
                                    if (thread_local std::mt19937 cleanup_rng(std::random_device{}()); cleanup_rng() % 100 == 0) {
                                        listeners.erase(it++); // Remove inactive entry
                                    } else {
                                        it->second.active = false; // Just reset the flag
                                        ++it;
                                    }
                                }
                            }
                        }
                        isActive = false; // Reset for next frame
                    }
                }

                // Signal work is finished
                threadState.individualState[i].workReady = false;
                threadState.individualState[i].workFinished = true;
            }
        });
    }
}

Nebulite::Interaction::Invoke::~Invoke() {
    // Signal threads to stop
    threadState.stopFlag = true;

    // Wake up all threads and wait for them to finish
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        threadState.individualState[i].condition.notify_one();
        if (threadrunners[i].joinable()) {
            threadrunners[i].join();
        }
    }
}

//------------------------------------------
// Checks

bool Nebulite::Interaction::Invoke::checkRulesetLogicalCondition(std::shared_ptr<Ruleset> const& cmd, Core::RenderObject const* otherObj) {
    // Check if logical arg is as simple as just "1", meaning true
    if (cmd->logicalArg.isAlwaysTrue())
        return true;

    double const result = cmd->logicalArg.evalAsDouble(otherObj->getDoc());
    if (isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

bool Nebulite::Interaction::Invoke::checkRulesetLogicalCondition(std::shared_ptr<Ruleset> const& cmd) {
    // Use selfPtr as otherObj
    return checkRulesetLogicalCondition(cmd, cmd->selfPtr);
}

//------------------------------------------
// Interactions

void Nebulite::Interaction::Invoke::broadcast(std::shared_ptr<Ruleset> const& entry) {
    // Get index
    uint32_t const id_self = entry->id;
    uint32_t const threadIndex = id_self % THREADRUNNER_COUNT;

    // Insert into next frame's entries
    std::scoped_lock lock(broadcasted.entriesNextFrame[threadIndex].mutex);
    auto& [isActive, rulesets] = broadcasted.entriesNextFrame[threadIndex].Container[entry->topic][id_self];
    rulesets[entry->index].entry = entry;
    isActive = true;
}

void Nebulite::Interaction::Invoke::listen(Core::RenderObject* obj, std::string const& topic, uint32_t const& listenerId) {
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
                bool const pairStatus = checkRulesetLogicalCondition(entry, obj);
                listeners[listenerId] = BroadCastListenPair{entry, obj, pairStatus};
            }
        }
    }
}

//------------------------------------------
// Ruleset application

void Nebulite::Interaction::Invoke::applyFunctionCalls(std::shared_ptr<Ruleset> const& ruleset, Core::RenderObject const* Obj_self, Core::RenderObject const* Obj_other) const {
    // === Function Calls GLOBAL ===
    for (auto& entry : ruleset->functioncalls_global) {
        // replace vars
        std::string call = entry.eval(Obj_other->getDoc());

        // attach to task queue
        std::scoped_lock lock(taskQueue.mutex);
        taskQueue.ptr->emplace_back(call);
    }

    // === Function Calls LOCAL: SELF ===
    for (auto& entry : ruleset->functioncalls_self) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(Obj_other->getDoc());
        (void)Obj_self->parseStr(call);
    }

    // === Function Calls LOCAL: OTHER ===
    for (auto& entry : ruleset->functioncalls_other) {
        // replace vars
        std::string const call = __FUNCTION__ + entry.eval(Obj_other->getDoc());
        (void)Obj_other->parseStr(call);
    }
}

void Nebulite::Interaction::Invoke::applyRulesets(std::shared_ptr<Ruleset> const& entries_self, Core::RenderObject const* Obj_other) const {
    // References
    Core::RenderObject const* Obj_self = entries_self->selfPtr;

    // Update self, other and global
    for (auto& assignment : entries_self->assignments) {
        assignment.apply(Obj_self->getDoc(), Obj_other->getDoc());
    }

    // Apply function calls
    applyFunctionCalls(entries_self, Obj_self, Obj_other);
}

void Nebulite::Interaction::Invoke::applyRulesets(std::shared_ptr<Ruleset> const& entries_self) const {
    applyRulesets(entries_self, entries_self->selfPtr);
}

//------------------------------------------
// Update

void Nebulite::Interaction::Invoke::update() {
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

std::string Nebulite::Interaction::Invoke::evaluateStandaloneExpression(std::string const& input) const {
    Data::JSON* docSelf = this->emptyDoc; // no self context
    Data::JSON* docOther = this->emptyDoc; // no other context

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.eval(docOther);
}

std::string Nebulite::Interaction::Invoke::evaluateStandaloneExpression(std::string const& input, Core::RenderObject const* selfAndOther) const {
    // Expression is evaluated within a domain's context, use it as self and other
    Data::JSON* docSelf = selfAndOther->getDoc();
    Data::JSON* docOther = selfAndOther->getDoc();

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.eval(docOther);
}

// Double evaluation

double Nebulite::Interaction::Invoke::evaluateStandaloneExpressionAsDouble(std::string const& input) const {
    Data::JSON* docSelf = this->emptyDoc; // no self context
    Data::JSON* docOther = this->emptyDoc; // no other context

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.evalAsDouble(docOther);
}

double Nebulite::Interaction::Invoke::evaluateStandaloneExpressionAsDouble(std::string const& input, Core::RenderObject const* selfAndOther) const {
    // Expression is evaluated within a domain's context, use it as self and other
    Data::JSON* docSelf = selfAndOther->getDoc();
    Data::JSON* docOther = selfAndOther->getDoc();

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    return expr.evalAsDouble(docOther);
}

// Boolean evaluation

bool Nebulite::Interaction::Invoke::evaluateStandaloneExpressionAsBool(std::string const& input) const {
    Data::JSON* docSelf = this->emptyDoc; // no self context
    Data::JSON* docOther = this->emptyDoc; // no other context

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    double const result = expr.evalAsDouble(docOther);
    if (isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}

bool Nebulite::Interaction::Invoke::evaluateStandaloneExpressionAsBool(std::string const& input, Core::RenderObject const* selfAndOther) const {
    // Expression is evaluated within a domain's context, use it as self and other
    Data::JSON* docSelf = selfAndOther->getDoc();
    Data::JSON* docOther = selfAndOther->getDoc();

    // Parse string into Expression
    Logic::ExpressionPool expr;
    expr.parse(input, docSelf);
    double const result = expr.evalAsDouble(docOther);
    if (isnan(result)) {
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return std::abs(result) > std::numeric_limits<double>::epsilon();
}