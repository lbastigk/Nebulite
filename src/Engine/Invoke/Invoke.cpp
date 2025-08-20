#include "Invoke.h"
#include "RenderObject.h"
#include "StringHandler.h"
#include <limits>

Nebulite::Invoke::Invoke(Nebulite::JSON* globalDocPtr){
    global = globalDocPtr;
}


bool Nebulite::Invoke::isTrueGlobal(std::shared_ptr<Nebulite::InvokeEntry> cmd, Nebulite::RenderObject* otherObj) {
    //-----------------------------------------
    // Pre-Checks
    
    // If self and other are the same object, the global check is always false
    if(cmd->selfPtr == otherObj) return false;

    // Check if logical arg is as simple as just "1", meaning true
    if(cmd->logicalArg.getFullExpression() == "1") return true;

    // A logicalArg of "0" would never really be used in prod,
    // (only for errors or quick removals of invokes in debugging)
    // which is why this check -> return false is not done.

    //-----------------------------------------
    // Evaluation

    // Get result
    double result = cmd->logicalArg.evalAsDouble(otherObj->getDoc());

    // Check for result
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << cmd->logicalArg.getFullExpression() << std::endl;
        // A NaN-Result can happen if any variable resolved isnt a number, but a text
        // Under usual circumstances, this is easily avoidable
        // by designing the values to only be assigned a numeric value.

        // In case this happens, it might be helpful to set the logic to always false:
        // This way, the error log does not happen all the time.
        cmd->logicalArg.parse("0", docCache, cmd->selfPtr->getDoc(), global);

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

bool Nebulite::Invoke::isTrueLocal(std::shared_ptr<Nebulite::InvokeEntry> cmd) {
    // Check if logical arg is as simple as just "1", meaning true
    if(cmd->logicalArg.getFullExpression() == "1") return true;

    // Resolve logical statement, using self as context for other
    double result = cmd->logicalArg.evalAsDouble(cmd->selfPtr->getDoc());
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << cmd->logicalArg.getFullExpression() << ". Resetting to 0" << std::endl;
        cmd->logicalArg.parse("0", docCache, cmd->selfPtr->getDoc(), global);
        return false;
    }
    return result != 0.0;
}

void Nebulite::Invoke::broadcast(std::shared_ptr<Nebulite::InvokeEntry> toAppend){
    std::lock_guard<std::mutex> lock(entries_global_next_Mutex);
    
    // Skip entries with empty topics - they should be local only
    if (toAppend->topic.empty()) {
        std::cerr << "Warning: Attempted to broadcast entry with empty topic - skipping" << std::endl;
        return;
    }
    
    // Store the shared pointer directly - no ownership issues
    entries_global_next[toAppend->topic].push_back(toAppend);
}

void Nebulite::Invoke::listen(Nebulite::RenderObject* obj,std::string topic){
    std::lock_guard<std::mutex> lock(entries_global_Mutex);
    for (auto& entry : entries_global[topic]){
        if(isTrueGlobal(entry,obj)){
            std::lock_guard<std::mutex> lock(pairsMutex);

            // Check if there is any existing batch
            if (pairs_threadsafe.empty() || pairs_threadsafe.back().size() >= THREADED_MIN_BATCHSIZE) {
                // Create a new batch
                pairs_threadsafe.emplace_back(); // Add an empty vector as a new batch
            }

            // Add to the current batch (last vector)
            pairs_threadsafe.back().emplace_back(entry, obj);
        }
    }
}

void Nebulite::Invoke::updateValueOfKey(Nebulite::Assignment::Operation operation, const std::string& key, const std::string& valStr, Nebulite::JSON* doc){    
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Assignment::Operation::set:
            doc->set<std::string>(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Assignment::Operation::add:
            doc->set_add(key.c_str(),std::stod(valStr));
            break;
        case Nebulite::Assignment::Operation::multiply:
            doc->set_multiply(key.c_str(),std::stod(valStr));
            break;
        case Nebulite::Assignment::Operation::concat:
            doc->set_concat(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Assignment::Operation::null:
            std::cerr << "Assignment expression has null operation - skipping" << std::endl;
            break;
        default:
            std::cerr << "Unknown operation type! Enum value:" << (int)operation << std::endl;
            break;
    }
}

void Nebulite::Invoke::updateValueOfKey(Nebulite::Assignment::Operation operation, const std::string& key, double value, Nebulite::JSON* doc){    
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Assignment::Operation::set:
            doc->set<double>(key.c_str(),value);
            break;
        case Nebulite::Assignment::Operation::add:
            doc->set_add(key.c_str(),value);
            break;
        case Nebulite::Assignment::Operation::multiply:
            doc->set_multiply(key.c_str(),value);
            break;
        case Nebulite::Assignment::Operation::concat:
            doc->set_concat(key.c_str(),std::to_string(value).c_str());
            break;
        case Nebulite::Assignment::Operation::null:
            std::cerr << "Assignment expression has null operation - skipping" << std::endl;
            break;
        default:
            std::cerr << "Unknown operation type! Enum value:" << (int)operation << std::endl;
            break;
    }
}

void Nebulite::Invoke::updatePair(std::shared_ptr<Nebulite::InvokeEntry> entries_self, Nebulite::RenderObject* Obj_other) {
    // Each thread needs its own variable list:


    // Set References
    Nebulite::RenderObject* Obj_self = entries_self->selfPtr;
    
    Nebulite::JSON* doc_self = Obj_self->getDoc();
    Nebulite::JSON* doc_other = Obj_other->getDoc();

    // Update self, other and global
    for(auto& expr : entries_self->exprs){
        //------------------------
        // Check what to update
        
        Nebulite::JSON* toUpdate = nullptr;
        switch (expr.onType) {
        case Nebulite::Assignment::Type::Self:
            toUpdate = doc_self;
            break;
        case Nebulite::Assignment::Type::Other:
            toUpdate = doc_other;
            break;
        case Nebulite::Assignment::Type::Global:
            toUpdate = global;
            break;
        case Nebulite::Assignment::Type::null:
            std::cerr << "Assignment expression has null type - skipping" << std::endl;
            continue; // Skip this expression
        default:
            std::cerr << "Unknown assignment type: " << (int)expr.onType << std::endl;
            return; // Exit if unknown type
        }

        //------------------------
        // Update
        
        // Direct use of double-values
        // Tests show that this causes a slight mismatch. Perhaps due to floating-point precision issues.
        // However, the functionality is preserved.
        // It just means that previous tests may not perfectly align with expected outcomes.
        //*
        if(expr.expression.isReturnableAsDouble()){
            double resolved = expr.expression.evalAsDouble(doc_other);
            updateValueOfKey(expr.operation, expr.key, resolved, toUpdate);
        }
        else
        //*/
        {
            std::string resolved = expr.expression.eval(doc_other);
            updateValueOfKey(expr.operation, expr.key, resolved, toUpdate);
        }
    }

    // === Functioncalls GLOBAL ===
    for(auto& entry : entries_self->functioncalls_global){
        // replace vars
        std::string call = entry.eval(doc_other);

        // attach to task queue
        std::lock_guard<std::recursive_mutex> lock(tasks_lock);
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

void Nebulite::Invoke::updateLocal(std::shared_ptr<Nebulite::InvokeEntry> entries_self){
    updatePair(entries_self, entries_self->selfPtr);
}

void Nebulite::Invoke::clear(){
    // Commands - shared pointers will automatically clean up
    entries_global.clear();
    entries_global_next.clear();
    
    // Pairs from commands
    pairs_threadsafe.clear();
}


void Nebulite::Invoke::update() {

    // Swap in the new set of commands - shared pointers will clean up automatically
    entries_global.clear();
    entries_global.swap(entries_global_next);
    
    // Go through all true pairs and update them
    std::vector<std::thread> threads;
    for (auto& pairs_batch : pairs_threadsafe) {
        threads.emplace_back([this, pairs_batch]() {
            for (auto& pair : pairs_batch) {
                updatePair(pair.first, pair.second);
            }
        });
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // Cleanup
    pairs_threadsafe.clear();
}



// ==========================
// Resolve Vars
// ==========================

std::string Nebulite::Invoke::evaluateStandaloneExpression(const std::string& input) {
    Nebulite::JSON* self = this->emptyDoc;
    Nebulite::JSON* other = this->emptyDoc;
    Nebulite::JSON* global = this->global;

    // Parse string into Expression
    Nebulite::ExpressionPool expr;
    expr.parse(input, docCache, self, global);
    return expr.eval(other);
}

