#include "Interaction/Invoke.hpp"

#include "Core/GlobalSpace.hpp"
#include "Core/RenderObject.hpp"

//------------------------------------------
// Constructor / Destructor

Nebulite::Interaction::Invoke::Invoke(Nebulite::Core::GlobalSpace* globalSpace)
: global(globalSpace),
  docCache(globalSpace->getDocCache()),
  globalDoc(globalSpace->getDoc())
{   
    // Initialize synchronization primitives
    threadState.stopFlag = false;
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        threadState.individualState[i].workReady = false;
        threadState.individualState[i].workFinished = false;
        
        // Start worker threads
        threadrunners[i] = std::thread([this, i](){
            while (!threadState.stopFlag){
                // Wait for work to be ready
                std::unique_lock<std::mutex> lock(broadcasted.entriesThisFrame[i].mutex);
                threadState.individualState[i].condition.wait(lock, [this, i] { 
                    return threadState.individualState[i].workReady.load() || threadState.stopFlag.load(); 
                });
                
                if (threadState.stopFlag) break;
                
                // Process work
                for(auto& map_other: std::views::values(broadcasted.entriesThisFrame[i].Container)){
                    for(auto& [isActive, rulesets] : std::views::values(map_other)){
                        if(!isActive) continue; // Skip if self did not broadcast this frame
                        for(auto& [entry, listeners] : std::ranges::views::values(rulesets)){
                            for(auto it = listeners.begin(); it != listeners.end();){
                                if(it->second.active){
                                    applyRulesets(it->second.entry, it->second.Obj_other);
                                    it->second.active = false; // Reset for next frame
                                    ++it;
                                }
                                else {
                                    // Probabilistic cleanup of inactive listeners
                                    // Use thread_local RNG for better performance
                                    // Since inactive listeners are rare, this should be fine
                                    // Sort of self-regulating, as more inactive listeners lead to more frequent cleanups
                                    thread_local static std::mt19937 cleanup_rng(std::random_device{}());
                                    if(cleanup_rng() % 100 == 0){
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

Nebulite::Interaction::Invoke::~Invoke(){
    // Signal threads to stop
    threadState.stopFlag = true;
    
    // Wake up all threads and wait for them to finish
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        threadState.individualState[i].condition.notify_one();
        if (threadrunners[i].joinable()){
            threadrunners[i].join();
        }
    }
}

//------------------------------------------
// Checks

bool Nebulite::Interaction::Invoke::checkRulesetLogicalCondition(std::shared_ptr<Nebulite::Interaction::Ruleset> const& cmd, Nebulite::Core::RenderObject const* otherObj){
    // Check if logical arg is as simple as just "1", meaning true
    if(cmd->logicalArg.isAlwaysTrue()) return true;

    double const result = cmd->logicalArg.evalAsDouble(otherObj->getDoc());
    if(isnan(result)){
        // We consider NaN as false
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return result != 0.0;
}

bool Nebulite::Interaction::Invoke::checkRulesetLogicalCondition(std::shared_ptr<Nebulite::Interaction::Ruleset> const& cmd){
    // Use selfPtr as otherObj
    return checkRulesetLogicalCondition(cmd, cmd->selfPtr);
}

//------------------------------------------
// Interactions

void Nebulite::Interaction::Invoke::broadcast(std::shared_ptr<Nebulite::Interaction::Ruleset> const& entry){
    // Get index
    uint32_t const id_self = entry->id;
    uint32_t const threadIndex = id_self % THREADRUNNER_COUNT;
    
    // Insert into next frame's entries
    std::scoped_lock<std::mutex> lock(broadcasted.entriesNextFrame[threadIndex].mutex);
    auto& onTopicFromId = broadcasted.entriesNextFrame[threadIndex].Container[entry->topic][id_self];
    onTopicFromId.rulesets[entry->index].entry = entry;
    onTopicFromId.active = true;
}

void Nebulite::Interaction::Invoke::listen(Nebulite::Core::RenderObject* obj,std::string const& topic, uint32_t const& listenerId){
    for (auto &[container, mutex] : std::span(broadcasted.entriesThisFrame, THREADRUNNER_COUNT)){
        // Lock to safely read from broadcasted.entriesThisFrame
        std::scoped_lock<std::mutex> broadcastLock(mutex);
        
        // Check if any object has broadcasted on this topic
        auto topicIt = container.find(topic);
        if (topicIt == container.end()){
            continue; // No entries for this topic in this thread
        }
        
        for (auto& [id_self, onTopicFromId] : topicIt->second){
            // Skip if broadcaster and listener are the same object
            if (id_self == listenerId) continue;

            // Skip if inactive
            if(!onTopicFromId.active) continue;

            // For all rulesets under this broadcaster and topic
            for (auto& [entry, listeners] : std::ranges::views::values(onTopicFromId.rulesets)) {
                bool const pairStatus = checkRulesetLogicalCondition(entry, obj);
                listeners[listenerId] = BroadCastListenPair{entry, obj, pairStatus};
            }
        }
    }
}

//------------------------------------------
// Value sets

void Nebulite::Interaction::Invoke::setValueOfKey(Nebulite::Interaction::Logic::Assignment::Operation operation, std::string const& key, std::string const& valStr, Nebulite::Utility::JSON* target){    
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Interaction::Logic::Assignment::Operation::set:
            target->set<std::string>(key,valStr);
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::add:
            target->set_add(key.c_str(),std::stod(valStr));
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::multiply:
            target->set_multiply(key.c_str(),std::stod(valStr));
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::concat:
            target->set_concat(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::null:
            Nebulite::Utility::Capture::cerr() << "Assignment expression has null operation - skipping" << Nebulite::Utility::Capture::endl;
            break;
        default:
            Nebulite::Utility::Capture::cerr() << "Unknown operation type! Enum value:" << static_cast<int>(operation) << Nebulite::Utility::Capture::endl;
            break;
    }
}

void Nebulite::Interaction::Invoke::setValueOfKey(Nebulite::Interaction::Logic::Assignment::Operation operation, std::string const& key, double const& value, Nebulite::Utility::JSON* target){
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Interaction::Logic::Assignment::Operation::set:
            target->set<double>(key,value);
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::add:
            target->set_add(key.c_str(),value);
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::multiply:
            target->set_multiply(key.c_str(),value);
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::concat:
            target->set_concat(key.c_str(),std::to_string(value).c_str());
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::null:
            Nebulite::Utility::Capture::cerr() << "Assignment expression has null operation - skipping" << Nebulite::Utility::Capture::endl;
            break;
        default:
            Nebulite::Utility::Capture::cerr() << "Unknown operation type! Enum value:" << static_cast<int>(operation) << Nebulite::Utility::Capture::endl;
            break;
    }
}

void Nebulite::Interaction::Invoke::setValueOfKey(Nebulite::Interaction::Logic::Assignment::Operation operation,  std::string const& key, double const& value, double* target){
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Interaction::Logic::Assignment::Operation::set:
            *target = value;
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::add:
            *target += value;
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::multiply:
            *target *= value;
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::concat:
            Nebulite::Utility::Capture::cerr() << "Unsupported operation: concat. If you see this message, something is wrong with the deserialization process of an Invoke!" << Nebulite::Utility::Capture::endl;
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::null:
            Nebulite::Utility::Capture::cerr() << "Assignment expression has null operation - skipping" << Nebulite::Utility::Capture::endl;
            break;
        default:
            Nebulite::Utility::Capture::cerr() << "Unknown operation type! Enum value:" << static_cast<int>(operation) << Nebulite::Utility::Capture::endl;
            break;
    }
}

//------------------------------------------
// Ruleset application

void Nebulite::Interaction::Invoke::applyAssignment(Nebulite::Interaction::Logic::Assignment& assignment, Nebulite::Core::RenderObject const* Obj_self, Nebulite::Core::RenderObject const* Obj_other) const {
    //------------------------------------------
    // Check what the target document to apply the ruleset to is
    
    Nebulite::Utility::JSON* targetDocument = nullptr;
    switch (assignment.onType){
    case Nebulite::Interaction::Logic::Assignment::Type::Self:
        targetDocument = Obj_self->getDoc();
        break;
    case Nebulite::Interaction::Logic::Assignment::Type::Other:
        targetDocument = Obj_other->getDoc();
        break;
    case Nebulite::Interaction::Logic::Assignment::Type::Global:
        targetDocument = globalDoc;
        break;
    case Nebulite::Interaction::Logic::Assignment::Type::null:
        Nebulite::Utility::Capture::cerr() << "Assignment expression has null type - skipping" << Nebulite::Utility::Capture::endl;
        return; // Skip this expression
    default:
        Nebulite::Utility::Capture::cerr() << "Unknown assignment type: " << static_cast<int>(assignment.onType) << Nebulite::Utility::Capture::endl;
        return; // Exit if unknown type
    }

    //------------------------------------------
    // Update

    // If the expression is returnable as double, we can optimize numeric operations
    if(assignment.expression.isReturnableAsDouble()){
        double resolved = assignment.expression.evalAsDouble(Obj_other->getDoc());
        if(assignment.targetValuePtr != nullptr){
            setValueOfKey(assignment.operation, assignment.key, resolved, assignment.targetValuePtr);
        }
        else{
            // Target is not associated with a direct double pointer
            // Likely because the target is in document other
            double* target = nullptr;

            // Try to use unique id for quick access
            if(!assignment.targetKeyUniqueIdInitialized){
                // Initialize unique id
                assignment.targetKeyUniqueId = global->getUniqueId(assignment.key, Nebulite::Core::GlobalSpace::UniqueIdType::jsonKey);
                assignment.targetKeyUniqueIdInitialized = true;
            }

            // Try to use unique id for quick access
            if(assignment.targetKeyUniqueId < Nebulite::Utility::JSON::uidQuickCacheSize){
                target = targetDocument->get_uid_double_ptr(assignment.targetKeyUniqueId, assignment.key);
            }
            // Fallback to normal method via key to double pointer
            else{
                // Try to get a stable double pointer from the target document
                target = targetDocument->getStableDoublePointer(assignment.key);
            }
            
            if(target != nullptr){
                // Lock is needed here, otherwise we have race conditions, and the engine is no longer deterministic!
                std::scoped_lock<std::recursive_mutex> lock(targetDocument->lock());
                setValueOfKey(assignment.operation, assignment.key, resolved, target);
            }
            else{
                // Still not possible, fallback to using JSON's internal methods
                // This is slower, but should work in all cases
                // No lock needed here, as we use JSON's threadsafe methods
                setValueOfKey(assignment.operation, assignment.key, resolved, targetDocument);
            }
        }
    }
    // If not, we resolve as string and update that way
    else{
        std::string resolved = assignment.expression.eval(Obj_other->getDoc());
        setValueOfKey(assignment.operation, assignment.key, resolved, targetDocument);
    }
}

void Nebulite::Interaction::Invoke::applyFunctionCalls(Nebulite::Interaction::Ruleset& ruleset, Nebulite::Core::RenderObject *Obj_self, Nebulite::Core::RenderObject *Obj_other) const {
    // === Function Calls GLOBAL ===
    for(auto& entry : ruleset.functioncalls_global){
        // replace vars
        std::string call = entry.eval(Obj_other->getDoc());

        // attach to task queue
        std::scoped_lock<std::mutex> lock(taskQueue.mutex);
        taskQueue.ptr->emplace_back(call);
    }

    // === Function Calls LOCAL: SELF ===
    for(auto& entry : ruleset.functioncalls_self){
        // replace vars
        std::string call = entry.eval(Obj_other->getDoc());
        (void)Obj_self->parseStr(call);
    }

    // === Function Calls LOCAL: OTHER ===
    for(auto& entry : ruleset.functioncalls_other){
        // replace vars
        std::string call = entry.eval(Obj_other->getDoc());
        (void)Obj_other->parseStr(call);
    }
}

void Nebulite::Interaction::Invoke::applyRulesets(std::shared_ptr<Nebulite::Interaction::Ruleset> const& entries_self, Nebulite::Core::RenderObject* Obj_other) const {
    // References
    Nebulite::Core::RenderObject* Obj_self = entries_self->selfPtr;

    // Update self, other and global
    for(auto& assignment : entries_self->assignments){
        applyAssignment(assignment, Obj_self, Obj_other);
    }

    // Apply function calls
    applyFunctionCalls(*entries_self, Obj_self, Obj_other);
}

void Nebulite::Interaction::Invoke::applyRulesets(std::shared_ptr<Nebulite::Interaction::Ruleset> const& entries_self) const {
    applyRulesets(entries_self, entries_self->selfPtr);
}

//------------------------------------------
// Update

void Nebulite::Interaction::Invoke::update(){
    // Signal all worker threads to start processing
    for (auto &[condition, workReady, workFinished] : std::span(threadState.individualState, THREADRUNNER_COUNT)){
        workReady = true;
        workFinished = false;
        condition.notify_one();
    }
    
    // Wait for all threads to finish processing
    for (auto &[condition, workReady, workFinished] : std::span(threadState.individualState, THREADRUNNER_COUNT)){
        while (!workFinished.load()){
            std::this_thread::yield(); // Yield to avoid busy waiting
        }
    }

    // Swap the containers, preparing for the next frame
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        // No workers active -> no mutex lock needed
        std::swap(broadcasted.entriesThisFrame[i].Container, broadcasted.entriesNextFrame[i].Container);
    }
}

//------------------------------------------
// Standalone Expression Evaluation

std::string Nebulite::Interaction::Invoke::evaluateStandaloneExpression(std::string const& input) const {
    Nebulite::Utility::JSON* docSelf = this->emptyDoc;      // no self context
    Nebulite::Utility::JSON* docOther = this->emptyDoc;     // no other context
    Nebulite::Utility::JSON* docGlobal = this->globalDoc;

    // Parse string into Expression
    Nebulite::Interaction::Logic::ExpressionPool expr;
    expr.parse(input, docCache, docSelf, docGlobal);
    return expr.eval(docOther);
}

std::string Nebulite::Interaction::Invoke::evaluateStandaloneExpression(std::string const& input, Nebulite::Core::RenderObject const* selfAndOther) const {
    // Expression is evaluated within a domain's context, use it as self and other
    Nebulite::Utility::JSON* docSelf = selfAndOther->getDoc();
    Nebulite::Utility::JSON* docOther = selfAndOther->getDoc();
    Nebulite::Utility::JSON* docGlobal = this->globalDoc;

    // Parse string into Expression
    Nebulite::Interaction::Logic::ExpressionPool expr;
    expr.parse(input, docCache, docSelf, docGlobal);
    return expr.eval(docOther);
}

