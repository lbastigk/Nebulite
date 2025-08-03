#include "Invoke.h"
#include "RenderObject.h"   // Linked here instead of in .h file due to circular dependencies
#include "StringHandler.h"

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
    te_variable sgn_var = {"sgn",   (void*)expr_custom::sgn,            TE_FUNCTION1};
    vars.push_back(sgn_var);
}

// TODO: Allow logicalArg to be a vector of strings!
void Nebulite::Invoke::parseFromJSON(Nebulite::JSON& doc, std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_global, std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_local, Nebulite::RenderObject* self) {
    
    entries_global.clear();
    entries_local.clear();

    if (doc.memberCheck(keyName.renderObject.invokes) != Nebulite::JSON::KeyType::array) {
        std::cerr << "Invokes field is not an array!" << std::endl;
        return;
    }

    // Get size of entries
    uint32_t size = doc.memberSize(keyName.renderObject.invokes);
    if (size == 0) {
        // Object has no invokes
        return;
    }

    for (int i = 0; i < size; ++i) {
        std::string key = keyName.renderObject.invokes + "[" + std::to_string(i) + "]";

        // Get entry
        Nebulite::JSON entry;
        if(doc.memberCheck(key.c_str()) == Nebulite::JSON::KeyType::document) {
            entry = doc.get_subdoc(key.c_str());
        }
        else{
            // Is link to document
            std::string link = doc.get<std::string>(key.c_str(), "");
            std::string file = FileManagement::LoadFile(link);
            if (file.empty()) {
                std::cerr << "Failed to load invoke file: " << key << " : " << link << std::endl;
                continue;
            }
            entry.deserialize(file);
        }

        // Basic settings from entry
        Nebulite::Invoke::Entry invokeEntry;
        invokeEntry.topic = entry.get<std::string>("topic", "all");

        // Get logical argument
        if(entry.memberCheck("logicalArg") == Nebulite::JSON::KeyType::array){
            uint32_t logicalArgSize = entry.memberSize("logicalArg");
            for(uint32_t j = 0; j < logicalArgSize; ++j) {
                std::string logicalArgKey = "logicalArg[" + std::to_string(j) + "]";
                std::string logicalArg = entry.get<std::string>(logicalArgKey.c_str(), "");
                invokeEntry.logicalArg += "(" + logicalArg + ")";
                if (j < logicalArgSize - 1) {
                    invokeEntry.logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
                }
            }
        }
        else{
            // Assume simple value, string:
            invokeEntry.logicalArg = entry.get<std::string>("logicalArg", "0");
        }
        
        
        // Get expressions
        if (entry.memberCheck(keyName.invoke.exprVector) == Nebulite::JSON::KeyType::array) {
            uint32_t exprSize = entry.memberSize(keyName.invoke.exprVector);
            for (uint32_t j = 0; j < exprSize; ++j) {
                std::string exprKey = keyName.invoke.exprVector + "[" + std::to_string(j) + "]";

                // Get expression
                std::string expr = entry.get<std::string>(exprKey.c_str(), "");

                // Turn string into assignmentExpr with {onType, key, operation, value, valueContainsReference}
                Nebulite::Invoke::AssignmentExpression assignmentExpr;

                // needs to start with "self.", "other." or "global."
                std::string prefix;
                if (expr.starts_with(keyName.invoke.typeSelf + ".")) {
                    assignmentExpr.onType = Nebulite::Invoke::AssignmentExpression::Type::Self;
                    prefix = keyName.invoke.typeSelf + ".";
                } else if (expr.starts_with(keyName.invoke.typeOther + ".")) {
                    assignmentExpr.onType = Nebulite::Invoke::AssignmentExpression::Type::Other;
                    prefix = keyName.invoke.typeOther + ".";
                } else if (expr.starts_with(keyName.invoke.typeGlobal + ".")) {
                    assignmentExpr.onType = Nebulite::Invoke::AssignmentExpression::Type::Global;
                    prefix = keyName.invoke.typeGlobal + ".";
                } else {
                    // Invalid expression
                    assignmentExpr.onType = Nebulite::Invoke::AssignmentExpression::Type::null;
                    continue;
                }

                // Find the operator position in the full expression, set operation, key and value
                size_t pos = std::string::npos;
                if ((pos = expr.find("+=")) != std::string::npos) {
                    assignmentExpr.operation = Nebulite::Invoke::AssignmentExpression::Operation::add;
                    assignmentExpr.value = expr.substr(pos + 2);
                    assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
                } else if ((pos = expr.find("*=")) != std::string::npos) {
                    assignmentExpr.operation = Nebulite::Invoke::AssignmentExpression::Operation::multiply;
                    assignmentExpr.value = expr.substr(pos + 2);
                    assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
                } else if ((pos = expr.find("|=")) != std::string::npos) {
                    assignmentExpr.operation = Nebulite::Invoke::AssignmentExpression::Operation::concat;
                    assignmentExpr.value = expr.substr(pos + 2);
                    assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
                } else if ((pos = expr.find("=")) != std::string::npos) {
                    assignmentExpr.operation = Nebulite::Invoke::AssignmentExpression::Operation::set;
                    assignmentExpr.value = expr.substr(pos + 1);
                    assignmentExpr.key = expr.substr(prefix.length(), pos - prefix.length());
                } else {
                    std::cerr << "No operation found in expression: " << expr << std::endl;
                    continue;
                }

                // Remove whitespaces at start and end of key and value
                assignmentExpr.key = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(assignmentExpr.key));
                assignmentExpr.value = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(assignmentExpr.value));
                

                // Add assignmentExpr to invokeEntry
                invokeEntry.exprs.push_back(assignmentExpr);
            }
        }

        // Get function calls: GLOBAL, SELF, OTHER
        if (entry.memberCheck(keyName.invoke.functioncalls_global) == Nebulite::JSON::KeyType::array) {
            uint32_t funcSize = entry.memberSize(keyName.invoke.functioncalls_global);
            for (uint32_t j = 0; j < funcSize; ++j) {
                std::string funcKey = keyName.invoke.functioncalls_global + "[" + std::to_string(j) + "]";
                std::string funcCall = entry.get<std::string>(funcKey.c_str(), "");
                invokeEntry.functioncalls_global.push_back(funcCall);
            }
        }
        if (entry.memberCheck(keyName.invoke.functioncalls_self) == Nebulite::JSON::KeyType::array) {
            uint32_t funcSize = entry.memberSize(keyName.invoke.functioncalls_self);
            for (uint32_t j = 0; j < funcSize; ++j) {
                std::string funcKey = keyName.invoke.functioncalls_self + "[" + std::to_string(j) + "]";
                std::string funcCall = entry.get<std::string>(funcKey.c_str(), "");

                // The first arg has to be some reference of where the function is called
                // Global functions explicitly place the Binary name on the front in the global space
                // Here, we just reference "self" as the first argument
                if (!funcCall.starts_with("self ")) {
                    funcCall = "self " + funcCall;
                }
                invokeEntry.functioncalls_self.push_back(funcCall);
            }
        }
        if (entry.memberCheck(keyName.invoke.functioncalls_other) == Nebulite::JSON::KeyType::array) {
            uint32_t funcSize = entry.memberSize(keyName.invoke.functioncalls_other);
            for (uint32_t j = 0; j < funcSize; ++j) {
                std::string funcKey = keyName.invoke.functioncalls_other + "[" + std::to_string(j) + "]";
                std::string funcCall = entry.get<std::string>(funcKey.c_str(), "");

                // The first arg has to be some reference of where the function is called
                // Global functions explicitly place the Binary name on the front in the global space
                // Here, we just reference "other" as the first argument
                if (!funcCall.starts_with("other ")) {
                    funcCall = "other " + funcCall;
                }
                invokeEntry.functioncalls_other.push_back(funcCall);
            }
        }

        // Remove whitespaces at start and end from topic and logicalArg:
        invokeEntry.topic = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(invokeEntry.topic));
        invokeEntry.logicalArg = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(invokeEntry.logicalArg));

        // Make shared_ptr from invokeEntry
        auto invokeEntryPtr = std::make_shared<Nebulite::Invoke::Entry>(invokeEntry);
        invokeEntryPtr->selfPtr = self; // Set self pointer

        if(invokeEntryPtr->topic.empty()){
            // If topic is empty, it is a local invoke
            invokeEntryPtr->isGlobal = false; // Set isGlobal to false for local invokes
            entries_local.push_back(invokeEntryPtr);
        } else {
            entries_global.push_back(invokeEntryPtr);
        }
    }
}

bool Nebulite::Invoke::isTrueGlobal(const std::shared_ptr<Nebulite::Invoke::Entry>& cmd, Nebulite::RenderObject* otherObj) {
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

        //Example:
        /*
        Evaluated logic to NAN! Logic is: :erase(neq(0.131000, 0))*(10.000000)*( lt(510.000000,        113.100000 + 10.000000))*( lt(40.000000,        101.382503 + 10.000000))*( lt(113.100000,       510.000000  + 10.000000))*( lt(101.382503,       40.000000  + 10.000000))*(not(  lt(40.000000+10.000000 - 2,101.382503) + lt(101.382503+10.000000 - 2,40.000000)  )))
        */

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

bool Nebulite::Invoke::isTrueLocal(const std::shared_ptr<Nebulite::Invoke::Entry>& cmd) {
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
    for(const auto& expr : entries_self->exprs){
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
            std::string resolved = resolveVars(expr.value, self, other, global);
            updateValueOfKey(expr.operation, expr.key, resolved, toUpdate);
        }
        else{
            updateValueOfKey(expr.operation, expr.key, expr.value, toUpdate);
        } 
    }

    // === Functioncalls GLOBAL ===
    for(auto call : entries_self->functioncalls_global){
        // replace vars
        call = resolveVars(call, self, other, global);

        // attach to task queue
        std::lock_guard<std::recursive_mutex> lock(tasks_lock);
        tasks->emplace_back(call);
    }

    // === Functioncalls LOCAL: SELF ===
    for(auto call : entries_self->functioncalls_self){
        // replace vars
        call = resolveVars(call, self, other, global);
        (void)Obj_self->parseStr(call);
    }

    // === Functioncalls LOCAL: OTHER ===
    for(auto call : entries_self->functioncalls_other){
        // replace vars
        call = resolveVars(call, self, other, global);
        (void)Obj_other->parseStr(call);
    }
}

void Nebulite::Invoke::updateLocal(const std::shared_ptr<Nebulite::Invoke::Entry>& entries_self){

    Nebulite::RenderObject* Obj_self = entries_self->selfPtr;
    Nebulite::RenderObject* Obj_other = entries_self->selfPtr;

    JSON *self  = Obj_self->getDoc();
    JSON *other = Obj_other->getDoc();

    // Update self, other and global
    for(const auto& expr : entries_self->exprs){
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
            std::string resolved = resolveVars(expr.value, self, other, global);
            updateValueOfKey(expr.operation, expr.key, resolved, toUpdate);
        }
        else{
            updateValueOfKey(expr.operation, expr.key, expr.value, toUpdate);
        } 
    }

    // === Functioncalls GLOBAL ===
    for(auto call : entries_self->functioncalls_global){
        // replace vars
        call = resolveVars(call, self, other, global);

        // attach to task queue
        std::lock_guard<std::recursive_mutex> lock(tasks_lock);
        tasks->emplace_back(call);
    }

    // === Functioncalls LOCAL: SELF ===
    for(auto call : entries_self->functioncalls_self){
        // replace vars
        call = resolveVars(call, self, other, global);
        Obj_self->parseStr(call);
    }

    // === Functioncalls LOCAL: OTHER ===
    for(auto call : entries_self->functioncalls_other){
        // replace vars
        call = resolveVars(call, self, other, global);
        Obj_other->parseStr(call);
    }
}

void Nebulite::Invoke::clear(){
    // Commands
    entries_global.clear();
    entries_global_next.clear();

    // Pairs from commands
    pairs_threadsafe.clear();

    // Expressions
    exprTree.clear();
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

double Nebulite::Invoke::evaluateExpression(const std::string& expr) {
    int err;
    te_expr* compiled = nullptr;
    compiled = te_compile(expr.c_str(), vars.data(), vars.size(), &err);
    if (!compiled) {
        std::cerr << "te_compile encountered error " << err << " for: "<< expr <<", returning NAN..." << std::endl;
        return NAN;
    }
    double result =  te_eval(compiled);
    te_free(compiled);
    return result;
}

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
    if (inner.find(InvokeResolveKeyword) != std::string::npos) {
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

std::shared_ptr<Nebulite::Invoke::Node> Nebulite::Invoke::expressionToTree(const std::string& input) {
    Node root;
    std::vector<std::shared_ptr<Invoke::Node>> children;
    size_t pos = 0;
    bool hasVariables = false;
    std::string literalBuffer;

    while (pos < input.size()) {
        if (input[pos] == InvokeResolveKeyword && pos + 1 < input.size()) {
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
    if (children.size() == 1 && children[0]->type == Node::Type::Variable && input.starts_with(InvokeResolveKeywordWithOpenParanthesis) && input.back() == ')') {
        resultNode = Node{ Node::Type::Variable, children[0]->text, {} };
    } else if (input.starts_with(InvokeResolveKeyword) && input.back() == ')') {
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

std::string Nebulite::Invoke::castValue(const std::string& value, Node* nodeptr, Nebulite::JSON *doc) {
    switch (nodeptr->cast) {
        case Node::CastType::None:
            return doc->get<std::string>(nodeptr->key.c_str(), "0");
        case Node::CastType::Float:
            return std::to_string(doc->get<double>(nodeptr->key.c_str(),0.0));
        case Node::CastType::Int:
            return std::to_string(doc->get<int>(nodeptr->key.c_str(),0));
        default:
            std::cerr << "Unknown cast type: " << static_cast<int>(nodeptr->cast) << std::endl;
            return "0"; // Fallback value
    }
}

std::string Nebulite::Invoke::nodeVariableAccess(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
    switch (nodeptr->context) {
        //---------------------------------------------------
        // First 3 Types: Variables

        // Get value from right doc, depending on ContextType:
        // self[key]
        // other[key]
        // global[key]
        case Node::ContextType::Self:
            return castValue(nodeptr->text, nodeptr.get(), self);
        case Node::ContextType::Other:
            return castValue(nodeptr->text, nodeptr.get(), other);
        case Node::ContextType::Global:
            return castValue(nodeptr->text, nodeptr.get(), global);
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
    // A non-match to the ContextTypes shouldnt happen, meaning this function is incomplete:
    std::cerr << "Nebulite::Invoke::nodeVariableAccess functions switch operations return is incomplete! Please inform the maintainers." << std::endl;
    return "";
}

std::string Nebulite::Invoke::evaluateNode(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
    if (nodeptr == nullptr) {
        std::cerr << "Nebulite::Invoke::evaluateNode error: Parent is nullptr!" << std::endl;
        return "";
    }

    // Check if JSON references are invalid (e.g., not pointing to initialized docs)
    if (self == nullptr || other == nullptr || global == nullptr) {
        // Log error and fallback
        std::cerr << "Nebulite::Invoke::evaluateNode error: One passed doc is nullptr!" << std::endl;
        return "";
    }
    
    // Depending on the type of node, evaluate accordingly
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
    }
    // For safety, if there are any logic mistakes with no returns:
    std::cerr << "Nebulite::Invoke::evaluateNode functions switch operations return is incomplete! Please inform the maintainers." << std::endl;
    return "";
}
  
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

std::string Nebulite::Invoke::resolveGlobalVars(const std::string& input) {
    return resolveVars(input,&emptyDoc,&emptyDoc,global);
}

