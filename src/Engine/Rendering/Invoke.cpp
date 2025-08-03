#include "Invoke.h"
#include "RenderObject.h"   // Linked here instead of in .h file due to circular dependencies
#include "StringHandler.h"

Nebulite::Invoke::Invoke() : nodeHelper(this) {
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

void Nebulite::Invoke::JSONParseHelper::getFunctionCalls(Nebulite::JSON& entryDoc, Nebulite::Invoke::Entry& invokeEntry){
    // Get function calls: GLOBAL, SELF, OTHER
    if (entryDoc.memberCheck(keyName.invoke.functioncalls_global) == Nebulite::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(keyName.invoke.functioncalls_global);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = keyName.invoke.functioncalls_global + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");
            invokeEntry.functioncalls_global.push_back(funcCall);
        }
    }
    if (entryDoc.memberCheck(keyName.invoke.functioncalls_self) == Nebulite::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(keyName.invoke.functioncalls_self);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = keyName.invoke.functioncalls_self + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "self" as the first argument
            if (!funcCall.starts_with("self ")) {
                funcCall = "self " + funcCall;
            }
            invokeEntry.functioncalls_self.push_back(funcCall);
        }
    }
    if (entryDoc.memberCheck(keyName.invoke.functioncalls_other) == Nebulite::JSON::KeyType::array) {
        uint32_t funcSize = entryDoc.memberSize(keyName.invoke.functioncalls_other);
        for (uint32_t j = 0; j < funcSize; ++j) {
            std::string funcKey = keyName.invoke.functioncalls_other + "[" + std::to_string(j) + "]";
            std::string funcCall = entryDoc.get<std::string>(funcKey.c_str(), "");

            // The first arg has to be some reference of where the function is called
            // Global functions explicitly place the Binary name on the front in the global space
            // Here, we just reference "other" as the first argument
            if (!funcCall.starts_with("other ")) {
                funcCall = "other " + funcCall;
            }
            invokeEntry.functioncalls_other.push_back(funcCall);
        }
    }
}

bool Nebulite::Invoke::JSONParseHelper::getExpression(Nebulite::Invoke::AssignmentExpression& assignmentExpr, Nebulite::JSON& entry, int index){
    std::string exprKey = keyName.invoke.exprVector + "[" + std::to_string(index) + "]";

    // Get expression
    std::string expr = entry.get<std::string>(exprKey.c_str(), "");

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
        return false;
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
        return false;
    }

    return true;
}

std::string Nebulite::Invoke::JSONParseHelper::getLogicalArg(Nebulite::JSON& entry) {
    std::string logicalArg = "";
    if(entry.memberCheck("logicalArg") == Nebulite::JSON::KeyType::array){
        uint32_t logicalArgSize = entry.memberSize("logicalArg");
        for(uint32_t j = 0; j < logicalArgSize; ++j) {
            std::string logicalArgKey = "logicalArg[" + std::to_string(j) + "]";
            logicalArg += "(" + entry.get<std::string>(logicalArgKey.c_str(), "0") + ")";
            if (j < logicalArgSize - 1) {
                logicalArg += "*"; // Arguments in vector need to be all true: &-logic -> Multiplication
            }
        }
    }
    else{
        // Assume simple value, string:
        logicalArg = entry.get<std::string>("logicalArg", "0");
    }
    return logicalArg;
}

bool Nebulite::Invoke::JSONParseHelper::getInvokeEntry(Nebulite::JSON& doc, Nebulite::JSON& entry, int index) {
    std::string key = keyName.renderObject.invokes + "[" + std::to_string(index) + "]";
    if(doc.memberCheck(key.c_str()) == Nebulite::JSON::KeyType::document) {
        entry = doc.get_subdoc(key.c_str());
    }
    else{
        // Is link to document
        std::string link = doc.get<std::string>(key.c_str(), "");
        std::string file = FileManagement::LoadFile(link);
        if (file.empty()) {
            return false;
        }
        entry.deserialize(file);
    }
    return true;
}

void Nebulite::Invoke::parseFromJSON(Nebulite::JSON& doc, std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_global, std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_local, Nebulite::RenderObject* self) {
    entries_global.clear();
    entries_local.clear();

    // Check if doc is valid
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

    // Iterate through all entries
    for (int i = 0; i < size; ++i) {
        // Parse entry into separate JSON object
        Nebulite::JSON entry;
        if (!JSONParseHelper::getInvokeEntry(doc, entry, i)) {
            std::cerr << "Failed to get invoke entry at index " << i << std::endl;
            continue; // Skip this entry
        }

        // Parse into a structure
        Nebulite::Invoke::Entry invokeEntry;
        invokeEntry.topic = entry.get<std::string>("topic", "all");
        invokeEntry.logicalArg = JSONParseHelper::getLogicalArg(entry);

        // Remove whitespaces at start and end from topic and logicalArg:
        invokeEntry.topic = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(invokeEntry.topic));
        invokeEntry.logicalArg = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(invokeEntry.logicalArg));
        
        // Get expressions
        if (entry.memberCheck(keyName.invoke.exprVector) == Nebulite::JSON::KeyType::array) {
            uint32_t exprSize = entry.memberSize(keyName.invoke.exprVector);
            for (uint32_t j = 0; j < exprSize; ++j) {
                Nebulite::Invoke::AssignmentExpression assignmentExpr;
                if (JSONParseHelper::getExpression(assignmentExpr, entry, j)){
                    // Successfully parsed expression

                    // Remove whitespaces at start and end of key and value
                    assignmentExpr.key = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(assignmentExpr.key));
                    assignmentExpr.value = Nebulite::StringHandler::rstrip(Nebulite::StringHandler::lstrip(assignmentExpr.value));
                    
                    // Add assignmentExpr to invokeEntry
                    invokeEntry.exprs.push_back(assignmentExpr);
                }
            }
        }
        else {
            std::cerr << "No expressions found in entry at index " << i << std::endl;
            continue; // Skip this entry if no expressions are found
        }

        // Parse all function calls
        Nebulite::Invoke::JSONParseHelper::getFunctionCalls(entry, invokeEntry);

        // Make shared_ptr from invokeEntry, push to vector
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


// ==========================
// Node Helper Functions
// ==========================

void Nebulite::Invoke::NodeHelper::foldConstants(const std::shared_ptr<Invoke::Node>& node) {
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
                std::string evaluated = std::to_string(invoke->evaluateExpression(combinedExpr));
                node->type = Node::Type::Literal;
                node->text = evaluated;
                node->children.clear(); // No need to keep children
            } catch (...) {
                // If evaluation fails, leave it as-is
            }
        }
    }
}


// Set context and optimization info of a variable node
//
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
// TODO: implementation in FileManagement class or Nebulite Namespace
// absl_flat_hash_map<std::string,Nebulite::JSON>
// if file doesnt exist, open
// Perhaps some more guards to close a file after n many updates if not used
Nebulite::Invoke::Node Nebulite::Invoke::NodeHelper::parseInnerVariable(const std::string& inner){
    Nebulite::Invoke::Node varNode = Node{ Node::Type::Variable, inner, {} };

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
    // - ...
    } else if (inner.starts_with("resources")){
        // For now, this ContextType is just set but not used
        varNode.context = Node::ContextType::Resources;
    }
    return varNode;
}

std::shared_ptr<Nebulite::Invoke::Node> Nebulite::Invoke::NodeHelper::parseChild(const std::string& input, size_t& i) {
    int depth = 1;
    size_t j = i;

    // Find the closing parenthesis for the current variable
    while (j < input.size() && depth > 0) {
        if (input[j] == '(') depth++;
        else if (input[j] == ')') depth--;
        ++j;
    }

    // If we reached the end of the string without finding a closing parenthesis, report an error
    if (depth != 0) {
        std::cerr << "Unmatched parentheses in expression: " << input << std::endl;
        return std::make_shared<Node>(Node{ Node::Type::Literal, input.substr(i, j - i), {} });
    }

    // Extract the inner content of the variable
    std::string inner = input.substr(i, j - i - 1); // inside $(...)
    Node varNode;

    // Check if string still contains some inner var to resolve:
    if (inner.find(InvokeResolveKeyword) != std::string::npos) {
        varNode = Node{ Node::Type::Mix_eval, "", { expressionToTree(inner) } };
    } 
    // Parse the inner variable
    else {
        varNode = parseInnerVariable(inner);
    }
    i = j; // move position after closing ')'
    return std::make_shared<Node>(varNode);
}

std::shared_ptr<Nebulite::Invoke::Node> Nebulite::Invoke::NodeHelper::expressionToTree(const std::string& input) {
    Node root;
    std::vector<std::shared_ptr<Invoke::Node>> children;
    size_t pos = 0;
    bool hasVariables = false;
    std::string literalBuffer;

    while (pos < input.size()) {
        // See if we have a variable to resolve
        bool childFound = false;
        Node::CastType castType = Node::CastType::None;
        if (input[pos] == InvokeResolveKeyword && pos + 1 < input.size()) {
            // No cast: $(...)
            if(input[pos + 1] == '('){
                pos += 2; // Skip "$("
                castType = Node::CastType::None;
                childFound = true;
            }
            // Int cast: $i(...)
            if(input[pos + 1] == 'i' && input[pos + 2] == '('){
                pos += 3; // Skip "$i("
                castType = Node::CastType::Int;
                childFound = true;
            }
            // float/double cast: $f(...)
            // While float casting is what's naturally done when evaluating with tinyexpr, this function might become useful
            // Perhaps when accessing some value that might or might not be a number, it is more convenient to directly cast to float
            // To avoid large expression strings. The implicit cast to float when accessing int variables might be helpful as well
            if(input[pos + 1] == 'f' && input[pos + 2] == '('){
                pos += 3; // Skip "$f("
                castType = Node::CastType::Float;
                childFound = true;
            }

            // If we found a child, parse it
            if(childFound){
                if (!literalBuffer.empty()) {
                    auto child = std::make_shared<Node>(Node{ Node::Type::Literal, literalBuffer, {} });
                    children.push_back(child);
                    literalBuffer.clear();
                }
                auto child = parseChild(input,pos);
                child->cast = castType;
                children.push_back(child);
                hasVariables = true;
                childFound = false;
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

std::string Nebulite::Invoke::NodeHelper::castValue(const std::string& value, Node* nodeptr, Nebulite::JSON *doc) {
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

std::string Nebulite::Invoke::NodeHelper::nodeVariableAccess(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
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
                if (nodeptr->cast == Node::CastType::Int) {
                    return std::to_string(std::stoi(nodeptr->text));
                }
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
                    return std::to_string(invoke->evaluateExpression(nodeptr->text));
                }
                if(nodeptr->cast == Node::CastType::Int){
                    return std::to_string((int)invoke->evaluateExpression(nodeptr->text));
                }
            }
            return std::to_string(invoke->evaluateExpression(nodeptr->text));
    }
    // A non-match to the ContextTypes shouldnt happen, meaning this function is incomplete:
    std::cerr << "Nebulite::Invoke::nodeVariableAccess functions switch operations return is incomplete! Please inform the maintainers." << std::endl;
    return "";
}

std::string Nebulite::Invoke::NodeHelper::combineChildren(const std::shared_ptr<Invoke::Node>& nodeptr, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global, bool insideEvalParent) {
    std::string result;
    for (const auto& child : nodeptr->children) {
        if (!child) {
            std::cerr << "Nebulite::Invoke::combineChildren error: Child node is nullptr!" << std::endl;
            continue;
        }
        result += evaluateNode(child, self, other, global, insideEvalParent);
    }
    return result;
}

std::string Nebulite::Invoke::NodeHelper::evaluateNode(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
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
        // Pure text, no variables or expressions
        case Node::Type::Literal:
            return nodeptr->text;
        // Variable access: self.variable, other.variable, global.variable
        case Node::Type::Variable:
            return nodeVariableAccess(nodeptr,self,other,global,insideEvalParent);
        // Mix no eval: "This string is a mix as it not only contains text, but also a variable access/expression like $(1+1)"
        case Node::Type::Mix_no_eval: {
            return combineChildren(nodeptr, self, other, global, false);
        }
        // Mix eval: "$(This string is a mix as it not only contains text, but also a variable access/expression like $(1+1))"
        case Node::Type::Mix_eval: {
            std::string combined = combineChildren(nodeptr, self, other, global, true);
            // Int casting
            if(nodeptr->cast == Node::CastType::Int){
                return std::to_string((int)invoke->evaluateExpression(combined));
            }
            // Float/None casting: directly evaluate and return
            return std::to_string(invoke->evaluateExpression(combined));
        }
    }
    // For safety, if there are any logic mistakes with no returns:
    std::cerr << "Nebulite::Invoke::evaluateNode functions switch operations return is incomplete! Please inform the maintainers." << std::endl;
    return "";
}
  
// ==========================
// Resolve Vars
// ==========================

std::string Nebulite::Invoke::resolveVars(const std::string& input, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global) {
    // Tree being used for resolving vars
    std::shared_ptr<Invoke::Node> tree;

    // Check if tree for this string already exists
    {
        std::unique_lock lock(exprTreeMutex);   // lock during check and insert
        auto it = exprTree.find(input);
        if (it == exprTree.end()) {
            tree = nodeHelper.expressionToTree(input);     // build outside the map
            exprTree[input] = tree;
        } else {
            tree = it->second;
        }
    }

    // Evaluate
    return nodeHelper.evaluateNode(tree, self, other, global, false);
}

std::string Nebulite::Invoke::resolveGlobalVars(const std::string& input) {
    return resolveVars(input,&emptyDoc,&emptyDoc,global);
}

