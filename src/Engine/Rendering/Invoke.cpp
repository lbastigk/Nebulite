#include "Invoke.h"
#include "RenderObject.h"  // include full definition of RenderObject


Nebulite::Invoke::Invoke(){
    // Manually add function variables
    te_variable gt_var = {"gt",     (void*)expr_custom::gt,             TE_FUNCTION2};
    vars.push_back(gt_var);
    te_variable lt_var = {"lt",     (void*)expr_custom::lt,             TE_FUNCTION2};
    vars.push_back(lt_var);
    te_variable geq_var = {"geq",   (void*)expr_custom::geq,            TE_FUNCTION2};
    vars.push_back(geq_var);
    te_variable leq_var = {"leq",   (void*)expr_custom::leq,            TE_FUNCTION2};
    vars.push_back(leq_var);
    te_variable eq_var = {"eq",     (void*)expr_custom::eq,             TE_FUNCTION2};
    vars.push_back(eq_var);
    te_variable neq_var = {"neq",   (void*)expr_custom::neq,            TE_FUNCTION2};
    vars.push_back(neq_var);
    te_variable and_var = {"and",   (void*)expr_custom::logical_and,    TE_FUNCTION2};
    vars.push_back(and_var);
    te_variable or_var = {"or",     (void*)expr_custom::logical_or,     TE_FUNCTION2};
    vars.push_back(or_var);
    te_variable not_var = {"not",   (void*)expr_custom::logical_not,    TE_FUNCTION1};
    vars.push_back(not_var);
}


bool Nebulite::Invoke::isTrueGlobal(const std::shared_ptr<InvokeEntry>& cmd, const std::shared_ptr<RenderObject>& otherObj) {
    // Resolve logical statement
    std::string logic = resolveVars(cmd->logicalArg, *cmd->selfPtr->getDoc(), *otherObj->getDoc(), *global);
    double result = evaluateExpression(logic);
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << logic << std::endl;
        return false;
    }
    return result != 0.0;
}

bool Nebulite::Invoke::isTrueLocal(const std::shared_ptr<InvokeEntry>& cmd) {
    // Resolve logical statement
    std::string logic = resolveVars(cmd->logicalArg, *cmd->selfPtr->getDoc(), *cmd->selfPtr->getDoc(), *global);
    double result = evaluateExpression(logic);
    if(isnan(result)){
        std::cerr << "Evaluated logic to NAN! Logic is: " << logic << std::endl;
        return false;
    }
    return result != 0.0;
}


// Checks an object against all linked invokes.
// True pairs are put into a special vector
void Nebulite::Invoke::checkAgainstList(const std::shared_ptr<RenderObject>& obj,std::string topic){
    for (auto& cmd : globalcommands[topic]){
        pairs.push_back(std::make_pair(cmd,obj));
    }
}


void Nebulite::Invoke::updateValueOfKey(const std::string& type, const std::string& key, const std::string& valStr, Nebulite::JSON *doc){
    if        (type == "set")       doc->set<std::string>(key.c_str(),valStr);
    else if   (type == "add")       doc->set<std::string>(key.c_str(),std::to_string(std::stod(valStr) + doc->get<double>(key.c_str(),0)));
    else if   (type == "multiply")  doc->set<std::string>(key.c_str(),std::to_string(std::stod(valStr) * doc->get<double>(key.c_str(),0)));
    else if   (type == "concat")    doc->set<std::string>(key.c_str(),doc->get<std::string>(key.c_str(),0) + valStr);
    // Fallback to set
    else                            doc->set<std::string>(key.c_str(),valStr);
}

// Checks a given invoke cmd against objects in buffer
// as objects have constant pointers, using RenderObject& is possible
void Nebulite::Invoke::updateGlobal(const std::shared_ptr<InvokeEntry>& cmd, const std::shared_ptr<RenderObject>& Obj) {
    // === SELF update ===
    for(auto InvokeTriple : cmd->invokes_self){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *Obj->getDoc(), *cmd->selfPtr->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, Obj->getDoc());
        } 
    }

    // === OTHER update ===
    for(auto InvokeTriple : cmd->invokes_other){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *Obj->getDoc(), *cmd->selfPtr->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === GLOBAL update ===
    for(auto InvokeTriple : cmd->invokes_global){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *Obj->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, global);
        } 
    }

    // === Functioncalls ===
    for(auto call : cmd->functioncalls){
        // replace vars
        call = resolveVars(call,*cmd->selfPtr->getDoc(),*Obj->getDoc(), *global);

        // attach to task queue
        tasks->emplace_back(call);
    }
}

void Nebulite::Invoke::updateLocal(const std::shared_ptr<InvokeEntry>& cmd){
    // === SELF update ===
    for(auto InvokeTriple : cmd->invokes_self){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *cmd->selfPtr->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === GLOBAL update ===
    for(auto InvokeTriple : cmd->invokes_global){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *cmd->selfPtr->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, global);
        } 
    }

    // === Functioncalls ===
    for(auto call : cmd->functioncalls){
        // replace vars
        call = resolveVars(call,*cmd->selfPtr->getDoc(),*cmd->selfPtr->getDoc(), *global);

        // attach to task queue
        tasks->emplace_back(call);
    }
}

void Nebulite::Invoke::clear(){
    globalcommands.clear();
    globalcommandsBuffer.clear();

    pairs.clear();
    pairs.shrink_to_fit();

    // TODO: Since exprTree holds a bunch of shared ptrs, the objects themselfes need to be cleared?
    // forall ptr in exprTree: free(ptr) ?
    // Since exprTree can hold more ptrs, this can be tricky...
    // For now it's not as important, as invokes dont necessary need to be cleared
    // But later on, constant level change might make this necessary
    // As for each env-deload an invoke clearing is reasonable
    exprTree.clear();
}

void Nebulite::Invoke::update(){
    for (auto pair : pairs){
        if(isTrueGlobal(pair.first,pair.second)){
            updateGlobal(pair.first, pair.second);
        }
    }
    pairs.clear();
    pairs.shrink_to_fit();
}

// Called after a full renderer update to get all extracted invokes
void Nebulite::Invoke::getNewInvokes(){
    globalcommands.clear();
    globalcommands.swap(globalcommandsBuffer);    // Swap in the new set of commands
}

void Nebulite::Invoke::append(const std::shared_ptr<InvokeEntry>& toAppend){
    globalcommandsBuffer[toAppend->topic].push_back(toAppend);
}

//--------------------------------------------------------
// TODO: usage of te_compile for faster evaluation...
// Idea is to use TE_FUNCTION2 by using maps: keyMap[d1], ptrMap[d2]
//
// double getKey(double d1, double d2){return *ptrMap[d2].get<double>(keyMap[d1],0.0);}
//
// Then, before te_compile each expr with $(self.key) is turned to: getKey(d1,d2)
// With the pointers being registered in pre-compiling
// This makes evaluatenode not necessary, instead a simple string precompile is done: 
// All references to self/other need to be resolved only, then everything still inside $(...) is send to evaluateExpression
// e.g.:
//  "The Value is: $($(self.key1) + $(other.key1) + 1)" 
//  -> 
//  "The Value is: " + std::to_string(evaluateExpression("getKey(<d1>,<d2>) + getKey(<d3>,<d4>) + 1"))
//
// The values for the maps should be fairly finite. The maximum is: 
//      ptrMap: MaxObjects                      key is double
//      keyMap: MaxKeysPerObject                key is double
//      ExpMap: MaxObjects*MaxKeysPerObject     key is string
// And should definetly be within memory limit. Positively, 
// in the string map no actual double values are stored so it cant grow superlarge
// Max for map[double] should be around 20mil values, due to precision. Start at -10.000.000 and go up 1 with each new entry
//
// Workflow:
//
// InvokeEntry.invokes_self[0] = "The Value is: $($(self.key1) + $(other.key1) + 1)"
// Recompile directly to:
// InvokeEntry.invokes_self[0] = "The Value is: " + std::to_string(evaluateExpression("getKey(<d1>,<d2>) + getKey(<d3>,<OTHER>) + 1"))
// Meaning there are special entries: 
//      - <OTHER> is replaced by other pointer on call
//      - <GLOBAL> is replaced by global pointer on call
//
// Important: Needs another workaround to allow for $(...) to deliver a string!
//
// Could mean lots of work, low priority for now...
// This would mean a big, BIG rework for a potentially unnecessary or marginal speedup
//
//--------------------------------------------------------

// Evaluating expression with already replaced self/other/global relations
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

        // Set context and optimization info
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

// Take a pre-processed node and resolve all expressions and vars of this and nodes below
//
// Examples:
// $($(global.constants.pi) + 1)  -> 4.141..
//   $(global.constants.pi) + 1   -> 3.141... + 1
// Time is: $(global.time.t)      -> Time is: 11.01
std::string Nebulite::Invoke::evaluateNode(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON& self,Nebulite::JSON& other,Nebulite::JSON& global,bool insideEvalParent){
    switch (nodeptr->type) {
        case Node::Type::Literal:
            return nodeptr->text;

        case Node::Type::Variable: {
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
                        return self.get<std::string>(nodeptr->key.c_str(), "0");
                    }
                    if(nodeptr->cast == Node::CastType::Float){
                        return std::to_string(self.get<double>(nodeptr->key.c_str(),0.0));
                    }
                    if(nodeptr->cast == Node::CastType::Int){
                        return std::to_string(self.get<int>(nodeptr->key.c_str(),0));
                    }
                case Node::ContextType::Other:
                    // Casting
                    if(nodeptr->cast == Node::CastType::None){
                        return other.get<std::string>(nodeptr->key.c_str(), "0");
                    }
                    if(nodeptr->cast == Node::CastType::Float){
                        return std::to_string(other.get<double>(nodeptr->key.c_str(),0.0));
                    }
                    if(nodeptr->cast == Node::CastType::Int){
                        return std::to_string(other.get<int>(nodeptr->key.c_str(),0));
                    }
                case Node::ContextType::Global:
                    // Casting
                    if(nodeptr->cast == Node::CastType::None){
                        return global.get<std::string>(nodeptr->key.c_str(), "0");
                    }
                    if(nodeptr->cast == Node::CastType::Float){
                        return std::to_string(global.get<double>(nodeptr->key.c_str(),0.0));
                    }
                    if(nodeptr->cast == Node::CastType::Int){
                        return std::to_string(global.get<int>(nodeptr->key.c_str(),0));
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
            }
        }

        // Mix-No-eval: "This string is a mix as it not only contains text, but also a variable access/expression like $(1+1)"
        case Node::Type::Mix_no_eval: {
            std::string result;
            for (auto& child : nodeptr->children) {
                result += evaluateNode(child, self, other, global, false);
            }
            return result;
        }

        // Mix eval: "$(This string is a mix as it not only contains text, but also a variable access/expression like $(1+1))"
        case Node::Type::Mix_eval: {
            std::string combined;
            for (auto& child : nodeptr->children) {
                combined += evaluateNode(child, self, other, global, true);
            }
            if(nodeptr->cast == Node::CastType::None || nodeptr->cast == Node::CastType::Float){
                return std::to_string(evaluateExpression(combined));
            }
            if(nodeptr->cast == Node::CastType::Int){
                return std::to_string((int)evaluateExpression(combined));
            }
        }
    }
    std::cerr << "Nebulite::Invoke::evaluateNode encountered a Node that did not resolve! This means there is a case where no return is given. Please inform the maintainers." << std::endl;
    std::cerr << "  Node type: " << static_cast<int>(nodeptr->type) << std::endl;
    std::cerr << "  Text: '" << nodeptr->text << "'" << std::endl;
    std::cerr << "  Children: " << nodeptr->children.size() << std::endl;
    std::cerr << "  Cast: " << static_cast<int>(nodeptr->cast) << std::endl;
    return "";
}
  
// replace all instances of $(...) with their evaluation
std::string Nebulite::Invoke::resolveVars(const std::string& input, Nebulite::JSON& self, Nebulite::JSON& other, Nebulite::JSON& global) {
    if(!exprTree.contains(input)){
        exprTree[input] = expressionToTree(input);
    }
    return evaluateNode(exprTree[input],self,other,global,false);
}

// same as resolveVars, but only using global, rest is empty
std::string Nebulite::Invoke::resolveGlobalVars(const std::string& input) {
    return resolveVars(input,emptyDoc,emptyDoc,*global);
}

