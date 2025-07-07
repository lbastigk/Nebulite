#include "Invoke.h"
#include "RenderObject.h"

//--------------------------------------------------
// TODO: 
// - vector-vector based threading as standard
// - mutex lock for functioncalls
//   then, all invokes are threadable
// - threadable local invokes
//   Inside Environment/ROC, reinsert batches for threading
//   but small batchsize, 10 perhaps?
// - local functioncalls for Renderobject:
//   each Renderobject with a tree to functions like:
//   - Position-text
//   - addChildren
//   - removeChildren
//   - reloadInvokes
//   - addInvoke
//   - removeAllInvokes
//--------------------------------------------------

Nebulite::Invoke::Invoke(){
    //-------------------------------------------------
    // Manually add function variables
    te_variable gt_var =  {"gt",    (void*)expr_custom::gt,             TE_FUNCTION2};
    vars.push_back(gt_var);
    te_variable lt_var =  {"lt",    (void*)expr_custom::lt,             TE_FUNCTION2};
    vars.push_back(lt_var);
    te_variable geq_var = {"geq",   (void*)expr_custom::geq,            TE_FUNCTION2};
    vars.push_back(geq_var);
    te_variable leq_var = {"leq",   (void*)expr_custom::leq,            TE_FUNCTION2};
    vars.push_back(leq_var);
    te_variable eq_var =  {"eq",    (void*)expr_custom::eq,             TE_FUNCTION2};
    vars.push_back(eq_var);
    te_variable neq_var = {"neq",   (void*)expr_custom::neq,            TE_FUNCTION2};
    vars.push_back(neq_var);
    te_variable and_var = {"and",   (void*)expr_custom::logical_and,    TE_FUNCTION2};
    vars.push_back(and_var);
    te_variable or_var =  {"or",    (void*)expr_custom::logical_or,     TE_FUNCTION2};
    vars.push_back(or_var);
    te_variable not_var = {"not",   (void*)expr_custom::logical_not,    TE_FUNCTION1};
    vars.push_back(not_var);
}

bool Nebulite::Invoke::isTrueGlobal(const std::shared_ptr<InvokeEntry>& cmd, const std::shared_ptr<RenderObject>& otherObj) {
    //-----------------------------------------
    // Pre-Checks
    
    // If self and other are the same object, the global check is always false
    if(cmd->selfPtr == otherObj) return false;

    // Check if logical arg is as simple as just "1", meaning true
    if(cmd->logicalArg == "1") return true;

    // A logicalArg of "0" would never really be used in prod,
    // (only for errors or quick removals of invokes in debugging)
    // which is why this check -> return false is not done.

    //-----------------------------------------
    // Evaluation

    // Resolve variables in expression
    std::string logic = resolveVars(cmd->logicalArg, cmd->selfPtr->getDoc(), otherObj->getDoc(), global);

    // Get result
    double result = evaluateExpression(logic);

    // Check for result
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << logic << std::endl;
        // A NaN-Result can happen if any variable resolved isnt a number, but a text
        // Under usual circumstances, this is easily avoidable
        // by designing the values to only be assigned a numeric value.

        // In case this happens, it might be helpful to set the logic to always false:
        // This way, the error log does not happen all the time.
        cmd->logicalArg = "0";

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

// Same as isTrue, but using self for other
// Might be helpful to use an empty doc here to supress any value from other being true
bool Nebulite::Invoke::isTrueLocal(const std::shared_ptr<InvokeEntry>& cmd) {
    // Chekc if logical arg is as simple as just "1", meaning true
    if(cmd->logicalArg == "1") return true;

    // Resolve logical statement
    std::string logic = resolveVars(cmd->logicalArg, cmd->selfPtr->getDoc(), cmd->selfPtr->getDoc(), global);
    double result = evaluateExpression(logic);
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << logic << std::endl;
        return false;
    }
    return result != 0.0;
}

// Broadcast an invoke to other renderobjects to listen
// Comparable to a radio, broadcasting on certain frequency determined by the string topic
void Nebulite::Invoke::broadcast(const std::shared_ptr<InvokeEntry>& toAppend){
    globalcommandsBuffer[toAppend->topic].push_back(toAppend);
}

// Listen to a certain topic
// TODO: is it better to do true/false check here instead of later?
// TODO: threadsafe checks are outdated, should work with more options now
void Nebulite::Invoke::listen(const std::shared_ptr<RenderObject>& obj,std::string topic){
    for (auto& cmd : globalcommands[topic]){
        // Determine if threadsafe or not

        // - no functioncalls
        // - no global invokeEntries
        // - only either self or other is manipulated
        //   technically, if both self and other are empty, this works as well
        //   though this wouldnt exist as this would be an empty invoke

        #if INVOKE_THREADING_TYPE == 0
            pairs_not_threadsafe.push_back(std::make_pair(cmd,obj));
        #elif INVOKE_THREADING_TYPE == 1
            // Map-Vector Based
            if(cmd->threadSafeType != Nebulite::Invoke::InvokeEntry::ThreadSafeType::None){
                if(cmd->threadSafeType != Nebulite::Invoke::InvokeEntry::ThreadSafeType::Self){
                    // Use self as pointer
                    pairs_threadsafe[cmd->selfPtr].push_back(std::make_pair(cmd,obj));
                }else if (cmd->threadSafeType != Nebulite::Invoke::InvokeEntry::ThreadSafeType::Other){
                    // Use Other as pointer
                    pairs_threadsafe[obj].push_back(std::make_pair(cmd,obj));
                }
            }else{
                // Might add no-op invokes
                // Though this is more of a game-design issue and should still be added:
                // - an extra check for no-op might introduce bugs later on if features are added
                // - unnecessary load, as empty invokes are easily avoided in game design
                pairs_not_threadsafe.push_back(std::make_pair(cmd,obj));
            }

        #else
            // Vector-Vector Based
            if(cmd->threadSafeType != Nebulite::Invoke::InvokeEntry::ThreadSafeType::None){
                // Check if we have any existing batch
                if (pairs_threadsafe.empty() || pairs_threadsafe.back().size() >= THREADED_MIN_BATCHSIZE) {
                    // Create a new batch
                    pairs_threadsafe.emplace_back(); // Add an empty vector as a new batch
                }

                // Add to the current batch (last vector)
                pairs_threadsafe.back().emplace_back(cmd, obj);
                
            }else{
                // Might add no-op invokes, see above for INVOKE_THREADING_TYPE == 1
                pairs_not_threadsafe.push_back(std::make_pair(cmd,obj));
            }
        #endif
    }
}

// Call representing functions for modification type in order to safely access the document
void Nebulite::Invoke::updateValueOfKey(Nebulite::Invoke::InvokeTriple::ChangeType type, const std::string& key, const std::string& valStr, Nebulite::JSON *doc){
    // Using Threadsafe manipulation methods of the JSON class:
    switch (type){
        case Nebulite::Invoke::InvokeTriple::ChangeType::set:
            doc->set<std::string>(key.c_str(),valStr);
            break;
        case Nebulite::Invoke::InvokeTriple::ChangeType::add:
            doc->set_add(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Invoke::InvokeTriple::ChangeType::multiply:
            doc->set_multiply(key.c_str(),valStr.c_str());
            break;
        case Nebulite::Invoke::InvokeTriple::ChangeType::concat:
            doc->set_concat(key.c_str(),valStr.c_str());
            break;
        default:
            std::cerr << "Unknown key type! Enum value:" << (int)type << std::endl;
            break;
    }
}

// Runs all entries in an invoke with self and other given
void Nebulite::Invoke::updateGlobal(const std::shared_ptr<InvokeEntry>& cmd_self, const std::shared_ptr<RenderObject>& Obj_other) {

    // === SELF update ===
    for(auto InvokeTriple : cmd_self->invokes_self){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, cmd_self->selfPtr->getDoc(), Obj_other->getDoc(), global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd_self->selfPtr->getDoc() );
        } 
    }

    // === OTHER update ===
    for(auto InvokeTriple : cmd_self->invokes_other){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, cmd_self->selfPtr->getDoc(), Obj_other->getDoc(), global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, Obj_other->getDoc());
        } 
    }

    // === GLOBAL update ===
    for(auto InvokeTriple : cmd_self->invokes_global){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, cmd_self->selfPtr->getDoc(), Obj_other->getDoc(), global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, global);
        } 
    }

    // === Functioncalls ===
    for(auto call : cmd_self->functioncalls){
        // replace vars
        call = resolveVars(call, cmd_self->selfPtr->getDoc(), Obj_other->getDoc(), global);

        // attach to task queue
        tasks->emplace_back(call);
    }
}

// Same as updateGlobal, but without an other-object
// Self is used as reference to other.
void Nebulite::Invoke::updateLocal(const std::shared_ptr<InvokeEntry>& cmd_self){

    // === SELF update ===
    for(auto InvokeTriple : cmd_self->invokes_self){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, cmd_self->selfPtr->getDoc(), cmd_self->selfPtr->getDoc(), global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd_self->selfPtr->getDoc());
        } 
    }

    // === GLOBAL update ===
    for(auto InvokeTriple : cmd_self->invokes_global){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, cmd_self->selfPtr->getDoc(), cmd_self->selfPtr->getDoc(), global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, global);
        } 
    }

    // === Functioncalls ===
    for(auto call : cmd_self->functioncalls){
        // replace vars
        call = resolveVars(call, cmd_self->selfPtr->getDoc(), cmd_self->selfPtr->getDoc(), global);

        // attach to task queue
        tasks->emplace_back(call);
    }
}

void Nebulite::Invoke::clear(){
    // Commands
    globalcommands.clear();
    globalcommandsBuffer.clear();

    // Pairs from commands
    pairs_threadsafe.clear();
    pairs_not_threadsafe.clear();
    pairs_not_threadsafe.shrink_to_fit();

    // Expressions
    exprTree.clear();
}

// Updating pairs of invokes
void Nebulite::Invoke::updatePairs() {

    //----------------------------------------------------
    // [THREADED] Version 1.1: 
    // Map-vector-based threading with minimum batch size
    //----------------------------------------------------
    #if INVOKE_THREADING_TYPE == 1 && THREADED_MIN_BATCHSIZE > 0

    std::vector<std::thread> threads;

    
    std::vector<std::pair<std::shared_ptr<InvokeEntry>, std::shared_ptr<RenderObject>>> current_batch;
    int current_batchsize = 0;
    for (auto& pairs_batch : pairs_threadsafe) {
        for (auto& pair : pairs_batch.second) {
            current_batch.push_back(pair);
            ++current_batchsize;
            if (current_batchsize >= THREADED_MIN_BATCHSIZE) {
                // Launch a thread for this batch
                threads.emplace_back([this, batch = std::move(current_batch)]() mutable {
                    for (auto& pair : batch) {
                        //*
                        if (isTrueGlobal(pair.first, pair.second)) {
                            updateGlobal(pair.first, pair.second);
                        }
                        //*/
                    }
                });
                current_batch.clear();
                current_batchsize = 0;
            }
        }
    }
    // Process any remaining pairs in the last batch
    if (!current_batch.empty()) {
        threads.emplace_back([this, batch = std::move(current_batch)]() mutable {
            for (auto& pair : batch) {
                if (isTrueGlobal(pair.first, pair.second)) {
                    updateGlobal(pair.first, pair.second);
                }
            }
        });
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    #endif

    //----------------------------------------------------
    // [THREADED] Version 1.2: 
    // Map-vector-based threading, no minimum batch size
    //----------------------------------------------------
    #if INVOKE_THREADING_TYPE == 1 && THREADED_MIN_BATCHSIZE <= 0
    std::vector<std::thread> threads;
    for (auto& pairs_batch : pairs_threadsafe) {
        if (!pairs_batch.second.empty()) {
            // Make a local copy of the vector to ensure safe access
            auto batch_copy = pairs_batch.second;
            threads.emplace_back([this, batch_copy]() {
                for (auto& pair : batch_copy) {
                    if (isTrueGlobal(pair.first, pair.second)) {
                        updateGlobal(pair.first, pair.second);
                    }
                }
            });
        }
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    #endif

    //----------------------------------------------------
    // [THREADED] Version 2:
    // Automatic batch sizing
    //----------------------------------------------------
    #if INVOKE_THREADING_TYPE == 2
    std::vector<std::thread> threads;
    for (auto& pairs_batch : pairs_threadsafe) {
        threads.emplace_back([this, pairs_batch]() {
            for (auto& pair : pairs_batch) {
                if (isTrueGlobal(pair.first, pair.second)) {
                    updateGlobal(pair.first, pair.second);
                }
            }
        });
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    #endif
    
    // --- Sequential update for non-threadsafe pairs ---
    for (auto pair : pairs_not_threadsafe) {
        if (isTrueGlobal(pair.first, pair.second)) {
            updateGlobal(pair.first, pair.second);
        }
    }

    // Cleanup
    pairs_threadsafe.clear();
    pairs_not_threadsafe.clear();
    pairs_not_threadsafe.shrink_to_fit();
}

// Called after a full renderer update to get all extracted invokes from the buffer
void Nebulite::Invoke::getNewInvokes(){
    globalcommands.clear();
    globalcommands.swap(globalcommandsBuffer);    // Swap in the new set of commands
}

// Evaluating expression with already replaced self/other/global etc. relations
double Nebulite::Invoke::evaluateExpression(const std::string& expr) {

    // Variable access via tinyexpr is needed for cache...
    /*
    auto it = expr_cache.find(expr);
    te_expr* compiled = nullptr;

    if (it != expr_cache.end()) {
        compiled = it->second;
    } 
    else {
        int err;
        compiled = te_compile(expr.c_str(), vars.data(), vars.size(), &err);
        if (!compiled) {
            //std::cerr << "Parse error at position " << err << std::endl;
            return NAN;
        }
        expr_cache[expr] = compiled;
    }
    return te_eval(compiled);
    */

    int err;
    te_expr* compiled = nullptr;
    compiled = te_compile(expr.c_str(), vars.data(), vars.size(), &err);
    if (!compiled) {
        //std::cerr << "Parse error at position " << err << std::endl;
        return NAN;
    }
    double result =  te_eval(compiled);
    te_free(compiled);
    return result;
}

// turn nodes that hold constant to evaluate into text
// e.g. $(1+1) is turned into 2.000...
void Nebulite::Invoke::foldConstants(const std::shared_ptr<Invoke::Node>& node) {
    // Recurse into children first
    for (auto& child : node->children) {
        foldConstants(child);
    }

    // Check if we can fold this Mix_eval node
    if (node->type == Node::Type::Mix_eval) {
        bool allLiteral = true;
        std::string combinedExpr;
        for (const auto& child : node->children) {
            if (child->type != Node::Type::Literal) {
                allLiteral = false;
                break;
            }
            combinedExpr += child->text;
        }

        if (allLiteral) {
            // Try evaluating it
            try {
                std::string evaluated = std::to_string(evaluateExpression(combinedExpr));
                node->type = Node::Type::Literal;
                node->text = evaluated;
                node->children.clear(); // No need to keep children
            } catch (...) {
                // If evaluation fails, leave it as-is
            }
        }
    }
}

// Parsing helper function
std::shared_ptr<Nebulite::Invoke::Node> Nebulite::Invoke::parseNext(const std::string& input, size_t& i) {
    size_t start = i + 2; // Skip "$("
    int depth = 1;
    size_t j = start;
    while (j < input.size() && depth > 0) {
        if (input[j] == '(') depth++;
        else if (input[j] == ')') depth--;
        ++j;
    }

    if (depth != 0) {
        std::cerr << "Unmatched parentheses in expression: " << input << std::endl;
        return std::make_shared<Node>(Node{ Node::Type::Literal, input.substr(i, j - i), {} });
    }

    std::string inner = input.substr(start, j - start - 1); // inside $(...)
    Node varNode;

    // Check if string still contains some inner var to resolve:
    if (inner.find("$") != std::string::npos) {
        varNode = Node{ Node::Type::Mix_eval, "", { expressionToTree(inner) } };
    } else {
        varNode = Node{ Node::Type::Variable, inner, {} };

        //-------------------------------------
        // Set context and optimization info

        // self/other/global are read-write docs
        // - object positions
        // - current time
        // - general game state info
        if (inner.starts_with("self.")) {
            varNode.context = Node::ContextType::Self;
            varNode.key = inner.substr(5);
        } else if (inner.starts_with("other.")) {
            varNode.context = Node::ContextType::Other;
            varNode.key = inner.substr(6);
        } else if (inner.starts_with("global.")) {
            varNode.context = Node::ContextType::Global;
            varNode.key = inner.substr(7);
        } else if (StringHandler::isNumber(inner)) {
            varNode.isNumericLiteral = true;
        // Resources docs are read only
        // - dialogue data
        // - 
        } else if (inner.starts_with("resources")){
            // TODO:
            // Is Resources doc
            // global is technically a resources doc as well, but with write-access
            // and a copy is thus stored under state prefix
            //
            // Resources docs are read-only
            //
            // $(resources.dialogue-characterName.onGreeting.v1)
            // -> Link = ./Resources/dialogue/characterName.json
            // -> key  = onGreeting.v1
            varNode.context = Node::ContextType::Resources;

            // TODO: implementation in FileManagement class or Nebulite Namespace
            // absl_flat_hash_map<std::string,Nebulite::JSON>
            // if file doesnt exist, open
            // Perhaps some more goards to close a file after n many updates if not used

            // For now, this ContextType is just set but not used
        }
    }
    i = j; // move position after closing ')'
    return std::make_shared<Node>(varNode);
}

// Main function for turning an expression into a Node Tree
std::shared_ptr<Nebulite::Invoke::Node> Nebulite::Invoke::expressionToTree(const std::string& input) {
    Node root;
    std::vector<std::shared_ptr<Invoke::Node>> children;
    size_t pos = 0;
    bool hasVariables = false;
    std::string literalBuffer;

    while (pos < input.size()) {
        if (input[pos] == '$' && pos + 1 < input.size()) {
            // No cast:
            if(input[pos + 1] == '('){
                if (!literalBuffer.empty()) {
                    auto child = std::make_shared<Node>(Node{ Node::Type::Literal, literalBuffer, {} });
                    children.push_back(child);
                    literalBuffer.clear();
                }
                auto child = parseNext(input,pos);
                child->cast = Node::CastType::None;
                children.push_back(child);
                hasVariables = true;
                continue;
            }
            // Int cast:
            if(input[pos + 1] == 'i' && input[pos + 2] == '('){
                pos++;
                if (!literalBuffer.empty()) {
                    auto child = std::make_shared<Node>(Node{ Node::Type::Literal, literalBuffer, {} });
                    children.push_back(child);
                    literalBuffer.clear();
                }
                auto child = parseNext(input,pos);
                child->cast = Node::CastType::Int;
                children.push_back(child);
                hasVariables = true;
                continue;
            }
            // float/double cast:
            // While float casting is what's naturally done when evaluating with tinyexpr, this function might become useful
            // Perhaps when accessing some value that might or might not be a number, it is more convenient to directly cast to float
            // To avoid large expression strings. The implicit cast to float when accessing int variables might be helpful as well
            if(input[pos + 1] == 'f' && input[pos + 2] == '('){
                pos++;
                if (!literalBuffer.empty()) {
                    auto child = std::make_shared<Node>(Node{ Node::Type::Literal, literalBuffer, {} });
                    children.push_back(child);
                    literalBuffer.clear();
                }
                auto child = parseNext(input,pos);
                child->cast = Node::CastType::Float;
                children.push_back(child);
                hasVariables = true;
                continue;
            }
        }
        literalBuffer += input[pos];
        ++pos;
    }

    if (!literalBuffer.empty()) {
        children.push_back(std::make_shared<Node>(Node{ Node::Type::Literal, literalBuffer, {} }));
    }

    Node resultNode;
    if (children.size() == 1 && children[0]->type == Node::Type::Variable && input.starts_with("$(") && input.back() == ')') {
        resultNode = Node{ Node::Type::Variable, children[0]->text, {} };
    } else if (input.starts_with("$(") && input.back() == ')') {
        resultNode = Node{ Node::Type::Mix_eval, "", children };
    } else if (hasVariables) {
        resultNode = Node{ Node::Type::Mix_no_eval, "", children };
    } else {
        resultNode = Node{ Node::Type::Literal, input, {} };
    }

    // Fold constants directly before returning
    auto ptr = std::make_shared<Node>(std::move(resultNode));
    foldConstants(ptr);
    return ptr;
}

// Helper function for accessing a variable from self/other/global/Resources
std::string Nebulite::Invoke::nodeVariableAccess(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
    switch (nodeptr->context) {
    //---------------------------------------------------
    // First 3 Types: Variables

    // Get value from right doc, depending on ContextType:
    // self[key]
    // other[key]
    // global[key]
    case Node::ContextType::Self:
        // Casting
        if(nodeptr->cast == Node::CastType::None){
            return self->get<std::string>(nodeptr->key.c_str(), "0");
        }
        if(nodeptr->cast == Node::CastType::Float){
            return std::to_string(self->get<double>(nodeptr->key.c_str(),0.0));
        }
        if(nodeptr->cast == Node::CastType::Int){
            return std::to_string(self->get<int>(nodeptr->key.c_str(),0));
        }
    case Node::ContextType::Other:
        // Casting
        if(nodeptr->cast == Node::CastType::None){
            return other->get<std::string>(nodeptr->key.c_str(), "0");
        }
        if(nodeptr->cast == Node::CastType::Float){
            return std::to_string(other->get<double>(nodeptr->key.c_str(),0.0));
        }
        if(nodeptr->cast == Node::CastType::Int){
            return std::to_string(other->get<int>(nodeptr->key.c_str(),0));
        }
    case Node::ContextType::Global:
        // Casting
        if(nodeptr->cast == Node::CastType::None){
            return global->get<std::string>(nodeptr->key.c_str(), "0");
        }
        if(nodeptr->cast == Node::CastType::Float){
            return std::to_string(global->get<double>(nodeptr->key.c_str(),0.0));
        }
        if(nodeptr->cast == Node::CastType::Int){
            return std::to_string(global->get<int>(nodeptr->key.c_str(),0));
        }
    //---------------------------------------------------
    // String is not a variable
    case Node::ContextType::None:
        // Simple Number:
        if (nodeptr->isNumericLiteral) {
            return nodeptr->text;
        // Inside eval parent: No need to call evaluateExpression right now, if no cast was defined.
        // Instead, return "(<expr>)" so it is evaled higher up
        // However, if a cast is specified, the evaluation should still happen
        // Return pure string in paranthesis only if CastType is None
        } else if (insideEvalParent && nodeptr->cast == Node::CastType::None) {
                return "(" + nodeptr->text + ")";
        // If not, evaluate and return
        } else{
            if(nodeptr->cast == Node::CastType::None || nodeptr->cast == Node::CastType::Float){
                return std::to_string(evaluateExpression(nodeptr->text));
            }
            if(nodeptr->cast == Node::CastType::Int){
                return std::to_string((int)evaluateExpression(nodeptr->text));
            }
        }
        return std::to_string(evaluateExpression(nodeptr->text));
}
}

// Take a pre-processed node and resolve all expressions and vars of this and nodes below
//
// Examples:
// $($(global.constants.pi) + 1)  -> 4.141..
//   $(global.constants.pi) + 1   -> 3.141... + 1
// Time is: $(global.time.t)      -> Time is: 11.01
std::string Nebulite::Invoke::evaluateNode(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
    if (nodeptr == nullptr) {
        std::cerr << "Nebulite::Invoke::evaluateNode error: Parent is nullptr!" << std::endl;
        return "";
    }

    // Optional: check if JSON references are invalid (e.g., not pointing to initialized docs)
    if (self == nullptr || other == nullptr || global == nullptr) {
        // Log error and fallback
        std::cerr << "Nebulite::Invoke::evaluateNode error: One passed doc is nullptr!" << std::endl;
        return "";
    }
    
    switch (nodeptr->type) {

        case Node::Type::Literal:
            return nodeptr->text;
            break;

        case Node::Type::Variable:
            return nodeVariableAccess(nodeptr,self,other,global,insideEvalParent);
            break;

        // Mix-No-eval: "This string is a mix as it not only contains text, but also a variable access/expression like $(1+1)"
        case Node::Type::Mix_no_eval: {
            std::string result;
            for (auto& child : nodeptr->children) {
                if (!child) {
                    std::cerr << "Nebulite::Invoke::evaluateNode error: Child node is nullptr!" << std::endl;
                    continue;
                }
                result += evaluateNode(child, self, other, global, false);
            }
            return result;
            }
            break;

        // Mix eval: "$(This string is a mix as it not only contains text, but also a variable access/expression like $(1+1))"
        case Node::Type::Mix_eval: {
            std::string combined;
            for (auto& child : nodeptr->children) {
                if (!child) {
                    std::cerr << "Nebulite::Invoke::evaluateNode error: Child node is nullptr!" << std::endl;
                    continue;
                }
                combined += evaluateNode(child, self, other, global, true);
            }
            if(nodeptr->cast == Node::CastType::None || nodeptr->cast == Node::CastType::Float){
                return std::to_string(evaluateExpression(combined));
            }
            if(nodeptr->cast == Node::CastType::Int){
                return std::to_string((int)evaluateExpression(combined));
            }
            return std::to_string(evaluateExpression(combined));
            }
            break;

        default:
            // Any other value means memory corruption or similiar?
            std::cerr << "Nebulite::Invoke::evaluateNode encountered an unkown Node type! Please inform the maintainers." << std::endl;
            return "";
    }
    // For safety, if there are any logic mistakes with no returns:
    std::cerr << "Nebulite::Invoke::evaluateNode functions switch operations return is incomplete! Please inform the maintainers." << std::endl;
    return "";
}
  
// replace all instances of $(...) with their evaluation
std::string Nebulite::Invoke::resolveVars(const std::string& input, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global) {
    // Tree being used for resolving vars
    std::shared_ptr<Invoke::Node> tree;

    // Check if tree for this string already exists
    {
        std::unique_lock lock(exprTreeMutex);   // lock during check and insert
        auto it = exprTree.find(input);
        if (it == exprTree.end()) {
            tree = expressionToTree(input);     // build outside the map
            exprTree[input] = tree;
        } else {
            tree = it->second;
        }
    }

    // Evaluate
    return evaluateNode(tree, self, other, global, false);
}

// same as resolveVars, but only using global variables. Self and other are linked to empty docs
std::string Nebulite::Invoke::resolveGlobalVars(const std::string& input) {
    return resolveVars(input,&emptyDoc,&emptyDoc,global);
}

