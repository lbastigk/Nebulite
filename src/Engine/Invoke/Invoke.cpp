#include "Invoke.h"
#include "RenderObject.h"
#include "StringHandler.h"
#include <limits>

Nebulite::Invoke::Invoke(){}


bool Nebulite::Invoke::isTrueGlobal(const std::shared_ptr<Nebulite::Invoke::Entry>& cmd, Nebulite::RenderObject* otherObj) {
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
    double result = std::stod(evaluateExpressionFull(cmd->logicalArg, cmd->selfPtr->getDoc(), otherObj->getDoc(), global));

    // Check for result
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << cmd->logicalArg.getFullExpression() << std::endl;
        // A NaN-Result can happen if any variable resolved isnt a number, but a text
        // Under usual circumstances, this is easily avoidable
        // by designing the values to only be assigned a numeric value.

        //Example:
        /*
        Evaluated logic to NAN! Logic is: :erase(neq(0.131000, 0))*(10.000000)*( lt(510.000000,        113.100000 + 10.000000))*( lt(40.000000,        101.382503 + 10.000000))*( lt(113.100000,       510.000000  + 10.000000))*( lt(101.382503,       40.000000  + 10.000000))*(not(  lt(40.000000+10.000000 - 2,101.382503) + lt(101.382503+10.000000 - 2,40.000000)  )))
        */

        // In case this happens, it might be helpful to set the logic to always false:
        // This way, the error log does not happen all the time.
        cmd->logicalArg.clear();

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

bool Nebulite::Invoke::isTrueLocal(const std::shared_ptr<Nebulite::Invoke::Entry>& cmd) {
    // Check if logical arg is as simple as just "1", meaning true
    if(cmd->logicalArg.getFullExpression() == "1") return true;

    // Resolve logical statement
    double result = std::stod(evaluateExpressionFull(cmd->logicalArg, cmd->selfPtr->getDoc(), cmd->selfPtr->getDoc(), global));
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << cmd->logicalArg.getFullExpression() << std::endl;
        return false;
    }
    return result != 0.0;
}

void Nebulite::Invoke::broadcast(const std::shared_ptr<Nebulite::Invoke::Entry>& toAppend){
    std::lock_guard<std::mutex> lock(entries_global_next_Mutex);
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

void Nebulite::Invoke::updateValueOfKey(Nebulite::Invoke::AssignmentExpression::Operation operation, const std::string& key, const std::string& valStr, Nebulite::JSON *doc){    
    // Using Threadsafe manipulation methods of the JSON class:
    switch (operation){
        case Nebulite::Invoke::AssignmentExpression::Operation::set:
            doc->set<std::string>(key.c_str(),valStr);
            break;
        case Nebulite::Invoke::AssignmentExpression::Operation::add:
            doc->set_add(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Invoke::AssignmentExpression::Operation::multiply:
            doc->set_multiply(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Invoke::AssignmentExpression::Operation::concat:
            doc->set_concat(key.c_str(),valStr.c_str());
            break;
        default:
            std::cerr << "Unknown key type! Enum value:" << (int)operation << std::endl;
            break;
    }
}

void Nebulite::Invoke::updatePair(const std::shared_ptr<Nebulite::Invoke::Entry>& entries_self, Nebulite::RenderObject* Obj_other) {

    Nebulite::RenderObject* Obj_self = entries_self->selfPtr;

    JSON *self  = Obj_self->getDoc();
    JSON *other = Obj_other->getDoc();

    // Update self, other and global
    for(auto& expr : entries_self->exprs){
        // Check what to update
        JSON *toUpdate = nullptr;
        switch (expr.onType) {
        case Nebulite::Invoke::AssignmentExpression::Type::Self:
            toUpdate = self;
            break;
        case Nebulite::Invoke::AssignmentExpression::Type::Other:
            toUpdate = other;
            break;
        case Nebulite::Invoke::AssignmentExpression::Type::Global:
            toUpdate = global;
            break;
        default:
            std::cerr << "Unknown assignment type: " << (int)expr.onType << std::endl;
            return; // Exit if unknown type
        }

        // Update
        if(expr.valueContainsReference){
            std::string resolved = evaluateExpressionFull(expr.expression, self, other, global);
            updateValueOfKey(expr.operation, expr.key, resolved, toUpdate);
        }
        else{
            updateValueOfKey(expr.operation, expr.key, expr.value, toUpdate);
        } 
    }

    // === Functioncalls GLOBAL ===
    for(auto entry : entries_self->functioncalls_global){
        // replace vars
        std::string call = entry.eval(self, other, global);

        // attach to task queue
        std::lock_guard<std::recursive_mutex> lock(tasks_lock);
        tasks->emplace_back(call);
    }

    // === Functioncalls LOCAL: SELF ===
    for(auto entry : entries_self->functioncalls_self){
        // replace vars
        std::string call = entry.eval(self, other, global);
        (void)Obj_self->parseStr(call);
    }

    // === Functioncalls LOCAL: OTHER ===
    for(auto entry : entries_self->functioncalls_other){
        // replace vars
        std::string call = entry.eval(self, other, global);
        (void)Obj_other->parseStr(call);
    }
}

void Nebulite::Invoke::updateLocal(const std::shared_ptr<Nebulite::Invoke::Entry>& entries_self){

    Nebulite::RenderObject* Obj_self = entries_self->selfPtr;
    Nebulite::RenderObject* Obj_other = entries_self->selfPtr;

    JSON *self  = Obj_self->getDoc();
    JSON *other = Obj_other->getDoc();

    // Update self, other and global
    for(auto& expr : entries_self->exprs){
        // Check what to update
        JSON *toUpdate = nullptr;
        switch (expr.onType) {
        case Nebulite::Invoke::AssignmentExpression::Type::Self:
            toUpdate = self;
            break;
        case Nebulite::Invoke::AssignmentExpression::Type::Other:
            toUpdate = other;
            break;
        case Nebulite::Invoke::AssignmentExpression::Type::Global:
            toUpdate = global;
            break;
        default:
            std::cerr << "Unknown assignment type: " << (int)expr.onType << std::endl;
            return; // Exit if unknown type
        }

        // Update
        if(expr.valueContainsReference){
            std::string resolved = evaluateExpressionFull(expr.expression, self, other, global);
            updateValueOfKey(expr.operation, expr.key, resolved, toUpdate);
        }
        else{
            updateValueOfKey(expr.operation, expr.key, expr.value, toUpdate);
        } 
    }

    // === Functioncalls GLOBAL ===
    for(auto entry : entries_self->functioncalls_global){
        // replace vars
        std::string call = entry.eval(self, other, global);

        // attach to task queue
        std::lock_guard<std::recursive_mutex> lock(tasks_lock);
        tasks->emplace_back(call);
    }

    // === Functioncalls LOCAL: SELF ===
    for(auto entry : entries_self->functioncalls_self){
        // replace vars
        std::string call = entry.eval(self, other, global);
        Obj_self->parseStr(call);
    }

    // === Functioncalls LOCAL: OTHER ===
    for(auto entry : entries_self->functioncalls_other){
        // replace vars
        std::string call = entry.eval(self, other, global);
        Obj_other->parseStr(call);
    }
}

void Nebulite::Invoke::clear(){
    // Commands
    entries_global.clear();
    entries_global_next.clear();

    // Pairs from commands
    pairs_threadsafe.clear();
}

void Nebulite::Invoke::update() {

    // Swap in the new set of commands
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

std::string Nebulite::Invoke::evaluateExpressionFull(Nebulite::InvokeExpression& expr, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global) {
    return expr.eval(self, other, global);
}

std::string Nebulite::Invoke::evaluateExpression(const std::string& input) {
    // Parse string into InvokeExpression
    Nebulite::InvokeExpression expr;
    expr.parse(input, docCache);
    return evaluateExpressionFull(expr, &emptyDoc, &emptyDoc, global);
}

