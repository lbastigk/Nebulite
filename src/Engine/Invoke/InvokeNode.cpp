#include "InvokeNode.h"
#include "Invoke.h"

// Keywords for resolving: $(1+1) , $(global.time.t) , ...
#define InvokeResolveKeyword '$'
#define InvokeResolveKeywordWithOpenParanthesis "$("

// ==========================
// InvokeNode Helper Functions
// ==========================

void Nebulite::InvokeNodeHelper::foldConstants(const std::shared_ptr<InvokeNode>& InvokeNode) {
    // Recurse into children first
    for (auto& child : InvokeNode->children) {
        foldConstants(child);
    }

    // Check if we can fold this Mix_eval InvokeNode
    if (InvokeNode->type == InvokeNode::Type::Mix_eval) {
        bool allLiteral = true;
        std::string combinedExpr;
        for (const auto& child : InvokeNode->children) {
            if (child->type != InvokeNode::Type::Literal) {
                allLiteral = false;
                break;
            }
            combinedExpr += child->text;
        }

        if (allLiteral) {
            // Try evaluating it
            try {
                std::string evaluated = std::to_string(invoke->evaluateExpression(combinedExpr));
                InvokeNode->type = InvokeNode::Type::Literal;
                InvokeNode->text = evaluated;
                InvokeNode->children.clear(); // No need to keep children
            } catch (...) {
                // If evaluation fails, leave it as-is
            }
        }
    }
}


// Set context and optimization info of a variable InvokeNode
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
Nebulite::InvokeNode Nebulite::InvokeNodeHelper::parseInnerVariable(const std::string& inner){
    // Setup a default variable node
    Nebulite::InvokeNode varNode;
    varNode.type = InvokeNode::Type::Variable;
    varNode.text = ""; // Must be explicitly set later
    varNode.context = InvokeNode::ContextType::None; // Default context
    varNode.cast = InvokeNode::CastType::None; // Default cast type
    varNode.isNumericLiteral = false; // Default is not a numeric literal

    // self/other/global are read-write docs
    // - object positions
    // - current time
    // - general game state info
    if (inner.starts_with("self.")) {
        varNode.text = inner.substr(5); // Remove "self."
        varNode.context = InvokeNode::ContextType::Self;
    } else if (inner.starts_with("other.")) {
        varNode.text = inner.substr(6); // Remove "other."
        varNode.context = InvokeNode::ContextType::Other;
    } else if (inner.starts_with("global.")) {
        varNode.text = inner.substr(7); // Remove "global."
        varNode.context = InvokeNode::ContextType::Global;
    } else if (StringHandler::isNumber(inner)) {
        varNode.text = inner;
        varNode.isNumericLiteral = true;
    // Resources docs are read only
    // - dialogue data
    // - ...
    } else if (inner.starts_with("resources")){
        // TODO: Implement Resources doc handling
        // For now, this ContextType is just set but not used
        varNode.context = InvokeNode::ContextType::Resources;

        // Perhaps even starting with a link to the file?
        // Example: $(./Resources/dialogue/characterName.json:onGreeting.v1)
        // 1.) Access fileManagement to get Nebulite::JSON pointer to "./Resources/dialogue/characterName.json"
        // 2.) json->get("onGreeting.v1") to get the value
        // 3.) Fallback, if file does not exist/fileManagement cant load it. Perhaps returning an empty Document
        // 4.) json->get(key,fallBackValue) to get the value, if needed.
        // Best default value might be link:key so that the user can see what is missing
    }
    return varNode;
}

std::shared_ptr<Nebulite::InvokeNode> Nebulite::InvokeNodeHelper::parseChild(const std::string& input, size_t& i) {
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
        Nebulite::InvokeNode varNode;
        varNode.type = InvokeNode::Type::Literal;
        varNode.text = input.substr(i, j - i);
        return std::make_shared<InvokeNode>(varNode);
    }

    // Extract the inner content of the variable
    std::string inner = input.substr(i, j - i - 1); // inside $(...)
    InvokeNode varNode;

    // Check if string still contains some inner var to resolve:
    if (inner.find(InvokeResolveKeyword) != std::string::npos) {
        varNode.type = InvokeNode::Type::Mix_eval;
        varNode.text = "";
        varNode.children.push_back(expressionToTree(inner));
    } 
    // Parse the inner variable
    else {
        varNode = parseInnerVariable(inner);
    }
    i = j; // move position after closing ')'
    return std::make_shared<InvokeNode>(varNode);
}

std::shared_ptr<Nebulite::InvokeNode> Nebulite::InvokeNodeHelper::expressionToTree(const std::string& input) {
    InvokeNode root;
    std::vector<std::shared_ptr<InvokeNode>> children;
    size_t pos = 0;
    bool hasVariables = false;
    std::string literalBuffer;

    while (pos < input.size()) {
        // See if we have a variable to resolve
        bool childFound = false;
        InvokeNode::CastType castType = InvokeNode::CastType::None;
        if (input[pos] == InvokeResolveKeyword && pos + 1 < input.size()) {
            // No cast: $(...)
            if(input[pos + 1] == '('){
                pos += 2; // Skip "$("
                castType = InvokeNode::CastType::None;
                childFound = true;
            }
            // Int cast: $i(...)
            if(input[pos + 1] == 'i' && input[pos + 2] == '('){
                pos += 3; // Skip "$i("
                castType = InvokeNode::CastType::Int;
                childFound = true;
            }
            // float/double cast: $f(...)
            // While float casting is what's naturally done when evaluating with tinyexpr, this function might become useful
            // Perhaps when accessing some value that might or might not be a number, it is more convenient to directly cast to float
            // To avoid large expression strings. The implicit cast to float when accessing int variables might be helpful as well
            if(input[pos + 1] == 'f' && input[pos + 2] == '('){
                pos += 3; // Skip "$f("
                castType = InvokeNode::CastType::Float;
                childFound = true;
            }

            // If we found a child, parse it
            if(childFound){
                if (!literalBuffer.empty()) {
                    InvokeNode varNode{  InvokeNode::Type::Literal, literalBuffer, {} };
                    auto child = std::make_shared<InvokeNode>(varNode);
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
        InvokeNode literalNode { InvokeNode::Type::Literal, literalBuffer, {} };
        children.push_back(std::make_shared<InvokeNode>(literalNode));
    }

    InvokeNode resultNode;
    if (children.size() == 1 && children[0]->type == InvokeNode::Type::Variable && input.starts_with(InvokeResolveKeywordWithOpenParanthesis) && input.back() == ')') {
        resultNode = InvokeNode{ InvokeNode::Type::Variable, children[0]->text, {} };
    } else if (input.starts_with(InvokeResolveKeyword) && input.back() == ')') {
        resultNode = InvokeNode{ InvokeNode::Type::Mix_eval, "", children };
    } else if (hasVariables) {
        resultNode = InvokeNode{ InvokeNode::Type::Mix_no_eval, "", children };
    } else {
        resultNode = InvokeNode{ InvokeNode::Type::Literal, input, {} };
    }

    // Fold constants directly before returning
    auto ptr = std::make_shared<InvokeNode>(std::move(resultNode));
    foldConstants(ptr);
    return ptr;
}

std::string Nebulite::InvokeNodeHelper::castValue(const std::string& value, InvokeNode* nodeptr, Nebulite::JSON *doc) {
    switch (nodeptr->cast) {
        case InvokeNode::CastType::None:
            return doc->get<std::string>(nodeptr->text.c_str(), "0");
        case InvokeNode::CastType::Float:
            return std::to_string(doc->get<double>(nodeptr->text.c_str(),0.0));
        case InvokeNode::CastType::Int:
            return std::to_string(doc->get<int>(nodeptr->text.c_str(),0));
        default:
            std::cerr << "Unknown cast type: " << static_cast<int>(nodeptr->cast) << std::endl;
            return "0"; // Fallback value
    }
}

std::string Nebulite::InvokeNodeHelper::nodeVariableAccess(const std::shared_ptr<InvokeNode>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
    switch (nodeptr->context) {
        //---------------------------------------------------
        // First 3 Types: Variables

        // Get value from right doc, depending on ContextType:
        // self[key]
        // other[key]
        // global[key]
        case InvokeNode::ContextType::Self:
            return castValue(nodeptr->text, nodeptr.get(), self);
        case InvokeNode::ContextType::Other:
            return castValue(nodeptr->text, nodeptr.get(), other);
        case InvokeNode::ContextType::Global:
            return castValue(nodeptr->text, nodeptr.get(), global);
        //---------------------------------------------------
        // String is not a variable
        case InvokeNode::ContextType::None:
            // Simple Number:
            if (nodeptr->isNumericLiteral) {
                if (nodeptr->cast == InvokeNode::CastType::Int) {
                    return std::to_string(std::stoi(nodeptr->text));
                }
                return nodeptr->text;
            // Inside eval parent: No need to call evaluateExpression right now, if no cast was defined.
            // Instead, return "(<expr>)" so it is evaled higher up
            // However, if a cast is specified, the evaluation should still happen
            // Return pure string in paranthesis only if CastType is None
            } else if (insideEvalParent && nodeptr->cast == InvokeNode::CastType::None) {
                    return "(" + nodeptr->text + ")";
            // If not, evaluate and return
            } else{
                if(nodeptr->cast == InvokeNode::CastType::None || nodeptr->cast == InvokeNode::CastType::Float){
                    return std::to_string(invoke->evaluateExpression(nodeptr->text));
                }
                if(nodeptr->cast == InvokeNode::CastType::Int){
                    return std::to_string((int)invoke->evaluateExpression(nodeptr->text));
                }
            }
            return std::to_string(invoke->evaluateExpression(nodeptr->text));
    }
    // A non-match to the ContextTypes shouldnt happen, meaning this function is incomplete:
    std::cerr << "Nebulite::InvokeNodeHelper::nodeVariableAccess functions switch operations return is incomplete! Please inform the maintainers." << std::endl;
    return "";
}

std::string Nebulite::InvokeNodeHelper::combineChildren(const std::shared_ptr<InvokeNode>& nodeptr, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global, bool insideEvalParent) {
    std::string result;
    for (const auto& child : nodeptr->children) {
        if (!child) {
            std::cerr << "Nebulite::Invoke::combineChildren error: Child InvokeNode is nullptr!" << std::endl;
            continue;
        }
        result += evaluateNode(child, self, other, global, insideEvalParent);
    }
    return result;
}

std::string Nebulite::InvokeNodeHelper::evaluateNode(const std::shared_ptr<InvokeNode>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent){
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
    
    // Depending on the type of InvokeNode, evaluate accordingly
    switch (nodeptr->type) {
        // Pure text, no variables or expressions
        case InvokeNode::Type::Literal:
            return nodeptr->text;
        // Variable access: self.variable, other.variable, global.variable
        case InvokeNode::Type::Variable:
            return nodeVariableAccess(nodeptr,self,other,global,insideEvalParent);
        // Mix no eval: "This string is a mix as it not only contains text, but also a variable access/expression like $(1+1)"
        case InvokeNode::Type::Mix_no_eval: {
            return combineChildren(nodeptr, self, other, global, false);
        }
        // Mix eval: "$(This string is a mix as it not only contains text, but also a variable access/expression like $(1+1))"
        case InvokeNode::Type::Mix_eval: {
            std::string combined = combineChildren(nodeptr, self, other, global, true);
            // Int casting
            if(nodeptr->cast == InvokeNode::CastType::Int){
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
  