#include "Interaction/Invoke.hpp"

#include "Core/GlobalSpace.hpp"
#include "Core/RenderObject.hpp"
#include "Utility/StringHandler.hpp"
#include <limits>

//------------------------------------------
// Constructor / Destructor

Nebulite::Interaction::Invoke::Invoke(Nebulite::Core::GlobalSpace* globalSpace)
: docCache(globalSpace),
  global(globalSpace),
  globalDoc(globalSpace->getDoc())
{   
    // Initialize synchronization primitives
    threadState.stopFlag = false;
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++) {
        threadState.individualState[i].workReady = false;
        threadState.individualState[i].workFinished = false;
        
        // Start worker threads
        threadrunners[i] = std::thread([this, i]() {
            while (!threadState.stopFlag) {
                // Wait for work to be ready
                std::unique_lock<std::mutex> lock(broadcasted.pairings[i].mutex);
                threadState.individualState[i].condition.wait(lock, [this, i] { 
                    return threadState.individualState[i].workReady.load() || threadState.stopFlag.load(); 
                });
                
                if (threadState.stopFlag) break;
                
                // Process work
                for(auto& [id_self, map_other] : broadcasted.pairings[i].work){
                    for(auto& [id_other, map_pairs] : map_other){
                        for(auto& [idx_ruleset, pair] : map_pairs){
                            if(pair.active){
                                applyRulesets(pair.entry, pair.Obj_other);
                                pair.active = false; // Mark as inactive after processing
                            }
                        }
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

void Nebulite::Interaction::Invoke::clear(){
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        std::lock_guard<std::mutex> lock1(broadcasted.entriesThisFrame[i].mutex);
        broadcasted.entriesThisFrame[i].work.clear();
        std::lock_guard<std::mutex> lock2(broadcasted.entriesNextFrame[i].mutex);
        broadcasted.entriesNextFrame[i].work.clear();
        std::lock_guard<std::mutex> lock3(broadcasted.pairings[i].mutex);
        broadcasted.pairings[i].work.clear();
    }
}

//------------------------------------------
// Checks

bool Nebulite::Interaction::Invoke::checkRulesetLogicalCondition(std::shared_ptr<Nebulite::Interaction::Ruleset> cmd, Nebulite::Core::RenderObject* otherObj) {
    //------------------------------------------
    // Pre-Checks
    
    // If self and other are the same object, the global check is always false
    if(cmd->selfPtr == otherObj) return false;

    // Check if logical arg is as simple as just "1", meaning true
    std::string_view expr = cmd->logicalArg.getFullExpressionStringview();
    if(expr == "1") return true;

    // A logicalArg of "0" would never really be used in prod,
    // (only for errors or quick removals of invokes in debugging)
    // which is why this check -> return false is not done.

    //------------------------------------------
    // Evaluation

    // Get result
    double result = cmd->logicalArg.evalAsDouble(otherObj->getDoc());

    // Check for result
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << expr << std::endl;
        // A NaN-Result can happen if any variable resolved isnt a number, but a text
        // Under usual circumstances, this is easily avoidable
        // by designing the values to only be assigned a numeric value.

        // In case this happens, it might be helpful to set the logic to always false:
        // This way, the error log does not happen all the time.
        cmd->logicalArg.parse("0", docCache, cmd->selfPtr->getDoc(), globalDoc);

        // This can become an unwanted behavior if the following is done:
        // logicalArg = $(not($(global.states.xyz)))
        // Perhaps that variable is 1 for now, 
        // but some other routine sets it to a non-numeric string 
        // e.g.: "wating"
        // If we now set logicalArg to "0", it will remain 0 unless the invoke is reloaded by the RenderObject flag.

        // Still, this behavior might be useful as it essentially says "This invoke has encountered an error, supressing evaulation"
        return false;
    }
    // Any double-value unequal to 0 is seen as "true"
    return result != 0.0;
}

bool Nebulite::Interaction::Invoke::checkRulesetLogicalCondition(std::shared_ptr<Nebulite::Interaction::Ruleset> cmd) {
    // Check if logical arg is as simple as just "1", meaning true
    std::string_view expr = cmd->logicalArg.getFullExpressionStringview();
    if(expr == "1") return true;

    // Resolve logical statement, using self as context for other
    double result = cmd->logicalArg.evalAsDouble(cmd->selfPtr->getDoc());
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << expr << ". Resetting to 0" << std::endl;
        cmd->logicalArg.parse("0", docCache, cmd->selfPtr->getDoc(), globalDoc);
        return false;
    }
    return result != 0.0;
}

//------------------------------------------
// Interactions

void Nebulite::Interaction::Invoke::broadcast(std::shared_ptr<Nebulite::Interaction::Ruleset> toAppend){
    // Skip entries with empty topics - they should be local only
    if (toAppend->topic.empty()) {
        std::cerr << "Warning: Attempted to broadcast entry with empty topic - skipping" << std::endl;
        return;
    }
    // Get index
    uint32_t id_self = toAppend->id;
    uint32_t threadIndex = id_self % THREADRUNNER_COUNT;
    
    // Insert into next frame's entries
    std::lock_guard<std::mutex> lock(broadcasted.entriesNextFrame[threadIndex].mutex);
    broadcasted.entriesNextFrame[threadIndex].work[toAppend->topic].push_back(toAppend);
}

void Nebulite::Interaction::Invoke::listen(Nebulite::Core::RenderObject* obj,std::string topic, uint32_t listenerId){
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        // Lock to safely read from broadcasted.entriesThisFrame
        std::lock_guard<std::mutex> broadcastLock(broadcasted.entriesThisFrame[i].mutex);
        
        // Check if topic exists to avoid creating empty entries
        auto topicIt = broadcasted.entriesThisFrame[i].work.find(topic);
        if (topicIt == broadcasted.entriesThisFrame[i].work.end()) {
            continue; // No entries for this topic in this thread
        }
        
        for (auto& entry : topicIt->second){
            if(checkRulesetLogicalCondition(entry,obj)){
                // Get all IDs and indices needed
                uint32_t id_self = entry->id;
                uint32_t threadIndex = id_self % THREADRUNNER_COUNT;

                // Lock correct mutex and update the entry
                std::lock_guard<std::mutex> lock(broadcasted.pairings[threadIndex].mutex);
                broadcasted.pairings[threadIndex].work[id_self][listenerId][entry->index] = {entry,obj,true};
            }
        }
    }
}

//------------------------------------------
// Value sets

void Nebulite::Interaction::Invoke::setValueOfKey(Nebulite::Interaction::Logic::Assignment::Operation operation, const std::string& key, const std::string& valStr, Nebulite::Utility::JSON* target){    
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Interaction::Logic::Assignment::Operation::set:
            target->set<std::string>(key.c_str(),valStr.c_str());
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
            std::cerr << "Assignment expression has null operation - skipping" << std::endl;
            break;
        default:
            std::cerr << "Unknown operation type! Enum value:" << (int)operation << std::endl;
            break;
    }
}

void Nebulite::Interaction::Invoke::setValueOfKey(Nebulite::Interaction::Logic::Assignment::Operation operation, const std::string& key, double value, Nebulite::Utility::JSON* target){    
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Interaction::Logic::Assignment::Operation::set:
            target->set<double>(key.c_str(),value);
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
            std::cerr << "Assignment expression has null operation - skipping" << std::endl;
            break;
        default:
            std::cerr << "Unknown operation type! Enum value:" << (int)operation << std::endl;
            break;
    }
}

void Nebulite::Interaction::Invoke::setValueOfKey(Nebulite::Interaction::Logic::Assignment::Operation operation, const std::string& key, double value, double* target){    
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
            std::cerr << "Unsupported operation: concat. If you see this message, something is wrong with the deserialization process of an Invoke!" << std::endl;
            break;
        case Nebulite::Interaction::Logic::Assignment::Operation::null:
            std::cerr << "Assignment expression has null operation - skipping" << std::endl;
            break;
        default:
            std::cerr << "Unknown operation type! Enum value:" << (int)operation << std::endl;
            break;
    }
}

//------------------------------------------
// Ruleset application

void Nebulite::Interaction::Invoke::applyRulesets(std::shared_ptr<Nebulite::Interaction::Ruleset> entries_self, Nebulite::Core::RenderObject* Obj_other) {
    // Each thread needs its own variable list:

    // Set References
    Nebulite::Core::RenderObject* Obj_self = entries_self->selfPtr;
    
    Nebulite::Utility::JSON* doc_self = Obj_self->getDoc();
    Nebulite::Utility::JSON* doc_other = Obj_other->getDoc();

    // Update self, other and global
    for(auto& assignment : entries_self->assignments){
        //------------------------------------------
        // Check what to update
        
        Nebulite::Utility::JSON* targetDocument = nullptr;
        switch (assignment.onType) {
        case Nebulite::Interaction::Logic::Assignment::Type::Self:
            targetDocument = doc_self;
            break;
        case Nebulite::Interaction::Logic::Assignment::Type::Other:
            targetDocument = doc_other;
            break;
        case Nebulite::Interaction::Logic::Assignment::Type::Global:
            targetDocument = globalDoc;
            break;
        case Nebulite::Interaction::Logic::Assignment::Type::null:
            std::cerr << "Assignment expression has null type - skipping" << std::endl;
            continue; // Skip this expression
        default:
            std::cerr << "Unknown assignment type: " << (int)assignment.onType << std::endl;
            return; // Exit if unknown type
        }

        //------------------------------------------
        // Update

        // If the expression is returnable as double, we can optimize numeric operations
        if(assignment.expression.isReturnableAsDouble()){
            double resolved = assignment.expression.evalAsDouble(doc_other);
            if(assignment.targetValuePtr != nullptr){
                setValueOfKey(assignment.operation, assignment.key, resolved, assignment.targetValuePtr);
            }
            else{
                // Target is not associated with a direct double pointer
                // Likely because the target is in document other

                // Try to get a stable double pointer from the target document
                double* target = targetDocument->get_stable_double_ptr(assignment.key.c_str());
                if(target != nullptr){
                    std::lock_guard<std::recursive_mutex> lock(targetDocument->lock());
                    setValueOfKey(assignment.operation, assignment.key, resolved, target);
                }
                else{
                    // Still not possible, fallback to using JSON's internal methods
                    // This is slower, but should work in all cases
                    std::lock_guard<std::recursive_mutex> lock(targetDocument->lock());
                    setValueOfKey(assignment.operation, assignment.key, resolved, targetDocument);
                }
            }
        }
        // If not, we resolve as string and update that way
        else{
            std::string resolved = assignment.expression.eval(doc_other);
            setValueOfKey(assignment.operation, assignment.key, resolved, targetDocument);
        }
    }

    // === Functioncalls GLOBAL ===
    for(auto& entry : entries_self->functioncalls_global){
        // replace vars
        std::string call = entry.eval(doc_other);

        // attach to task queue
        std::lock_guard<std::recursive_mutex> lock(globalTasksLock);
        tasks->emplace_back(call);
    }

    // === Functioncalls LOCAL: SELF ===
    for(auto& entry : entries_self->functioncalls_self){
        // replace vars
        std::string call = entry.eval(doc_other);
        (void)Obj_self->parseStr(call);
    }

    // === Functioncalls LOCAL: OTHER ===
    for(auto& entry : entries_self->functioncalls_other){
        // replace vars
        std::string call = entry.eval(doc_other);
        (void)Obj_other->parseStr(call);
    }
}

void Nebulite::Interaction::Invoke::applyRulesets(std::shared_ptr<Nebulite::Interaction::Ruleset> entries_self){
    applyRulesets(entries_self, entries_self->selfPtr);
}

//------------------------------------------
// Update

void Nebulite::Interaction::Invoke::update() {
    // Swap in the new set of commands
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        std::lock_guard<std::mutex> lock1(broadcasted.entriesThisFrame[i].mutex);   // Shouldnt be necessary as no other process accesses this during update, but we are paranoid
        std::lock_guard<std::mutex> lock2(broadcasted.entriesNextFrame[i].mutex);   // Shouldnt be necessary as no other process accesses this during update, but we are paranoid
        broadcasted.entriesThisFrame[i].work = std::move(broadcasted.entriesNextFrame[i].work);
    }

    // Signal all worker threads to start processing
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        threadState.individualState[i].workFinished = false;
        threadState.individualState[i].workReady = true;
        threadState.individualState[i].condition.notify_one();
    }
    
    // Wait for all threads to finish processing
    for (size_t i = 0; i < THREADRUNNER_COUNT; i++){
        while (!threadState.individualState[i].workFinished.load()) {
            std::this_thread::yield(); // Yield to avoid busy waiting
        }
    }
}

//------------------------------------------
// Standalone Expression Evaluation

std::string Nebulite::Interaction::Invoke::evaluateStandaloneExpression(const std::string& input) {
    Nebulite::Utility::JSON* self = this->emptyDoc;
    Nebulite::Utility::JSON* other = this->emptyDoc;
    Nebulite::Utility::JSON* global = this->globalDoc;

    // Parse string into Expression
    Nebulite::Interaction::Logic::ExpressionPool expr;
    expr.parse(input, docCache, self, global);
    return expr.eval(other);
}

std::string Nebulite::Interaction::Invoke::evaluateStandaloneExpression(const std::string& input, Nebulite::Core::RenderObject* selfAndOther) {
    Nebulite::Utility::JSON* self = selfAndOther->getDoc();
    Nebulite::Utility::JSON* other = selfAndOther->getDoc();
    Nebulite::Utility::JSON* global = this->globalDoc;

    // Parse string into Expression
    Nebulite::Interaction::Logic::ExpressionPool expr;
    expr.parse(input, docCache, self, global);
    return expr.eval(other);
}

