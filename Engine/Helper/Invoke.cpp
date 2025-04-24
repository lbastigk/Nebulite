#include "Invoke.h"
#include "RenderObject.h"  // include full definition of RenderObject

// Needs to be linked to a doc containing global variables
Invoke::Invoke(rapidjson::Document& globalDocPtr) {
    global = &globalDocPtr;

    // Global table elements?
    expression.register_symbol_table(symbol_table);
}

// Checks a given invoke cmd against objects in buffer
// as objects have constant pointers, using RenderObject& is possible
void Invoke::check(std::shared_ptr<InvokeCommand> cmd, RenderObject& otherObj) {

    // Same send
    bool skipOther = false;
    if(cmd->selfPtr == &otherObj){
        if (cmd->otherChangeType.empty() && cmd->otherKey.empty() &&  cmd->otherValue.empty()){
            skipOther = true;
        }
        else{ 
            return;
        }
    }

    // Resolve logical statement
    std::string logic = resolveVars(cmd->logicalArg, *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
    if (evaluateExpression(logic) == 0.0){
        return;
    }

    // === SELF update ===
    if (!cmd->selfKey.empty() && !cmd->selfChangeType.empty()) {
        std::string valStr = resolveVars(cmd->selfValue, *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
        double val = evaluateExpression(valStr);
        double oldVal = JSONHandler::Get::Any<double>(*cmd->selfPtr->getDoc(), cmd->selfKey, 0.0);

        if (cmd->selfChangeType == "set") {
            cmd->selfPtr->valueSet<double>(cmd->selfKey, val);
        } else if (cmd->selfChangeType == "add") {
            cmd->selfPtr->valueSet<double>(cmd->selfKey, oldVal + val);
        } else if (cmd->selfChangeType == "multiply") {
            cmd->selfPtr->valueSet<double>(cmd->selfKey, oldVal * val);
        } else if (cmd->selfChangeType == "append") {
            std::string oldStr = JSONHandler::Get::Any<std::string>(*cmd->selfPtr->getDoc(), cmd->selfKey, "");
            cmd->selfPtr->valueSet<std::string>(cmd->selfKey, oldStr + valStr);
        }
    }


    // === OTHER update ===
    if (!skipOther) {
        std::string valStr = resolveVars(cmd->otherValue,  *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
        double val = evaluateExpression(valStr);
        double oldVal = JSONHandler::Get::Any<double>(*otherObj.getDoc(), cmd->otherKey, 0.0);

        if (cmd->otherChangeType == "set") {
            otherObj.valueSet<double>(cmd->otherKey, val);
        } else if (cmd->otherChangeType == "add") {
            otherObj.valueSet<double>(cmd->otherKey, oldVal + val);
        } else if (cmd->otherChangeType == "multiply") {
            otherObj.valueSet<double>(cmd->otherKey, oldVal * val);
        } else if (cmd->otherChangeType == "append") {
            std::string oldStr = JSONHandler::Get::Any<std::string>(*otherObj.getDoc(), cmd->otherKey, "");
            otherObj.valueSet<std::string>(cmd->otherKey, oldStr + valStr);
        }
    }

    // === GLOBAL update ===
    if (!cmd->globalKey.empty() && !cmd->globalChangeType.empty()) {
        std::string valStr = resolveVars(cmd->globalValue,  *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
        double val = evaluateExpression(valStr);
        double oldVal = JSONHandler::Get::Any<double>(*global, cmd->globalKey, 0.0);

        if (cmd->globalChangeType == "set") {
            JSONHandler::Set::Any<double>(*global, cmd->globalKey, val);
        } else if (cmd->globalChangeType == "add") {
            JSONHandler::Set::Any<double>(*global, cmd->globalKey, oldVal + val);
        } else if (cmd->globalChangeType == "multiply") {
            JSONHandler::Set::Any<double>(*global, cmd->globalKey, oldVal * val);
        } else if (cmd->globalChangeType == "append") {
            std::string oldStr = JSONHandler::Get::Any<std::string>(*global, cmd->globalKey, "");
            JSONHandler::Set::Any<std::string>(*global, cmd->globalKey, oldStr + valStr);
        }
    }
}

void Invoke::checkAgainstList(RenderObject& obj){
    for (auto& cmd : commands){
        check(cmd,obj);
    }
}
void Invoke::checkLoop(){
    loopCommands.clear();
    loopCommands.swap(nextLoopCommands);
    for (auto& cmd : loopCommands){
        check(cmd,*cmd->selfPtr);
    }
    
}


void Invoke::getNewInvokes(){
    commands.clear();
    // Swap in the new set of commands
    commands.swap(nextCommands);
}



void Invoke::append(std::shared_ptr<InvokeCommand> toAppend){
    if      (!toAppend->type.compare("continous"))   nextCommands.push_back(toAppend);
    else if (!toAppend->type.compare("loop"))        nextLoopCommands.push_back(toAppend);
}


double Invoke::evaluateExpression(const std::string& expr) {
    

    double result = 0.0;
    if (parser.compile(expr, expression)) {
        result = expression.value();
    }
    return result;
}

std::string Invoke::resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global) {
    std::string result = input;
    size_t pos = 0;
    while ((pos = result.find("$(", pos)) != std::string::npos) {
        size_t start = pos + 2;
        int depth = 1;
        size_t end = start;
        while (end < result.size() && depth > 0) {
            if (result[end] == '(') depth++;
            else if (result[end] == ')') depth--;
            ++end;
        }

        if (depth != 0) {
            // Unmatched parentheses
            break;
        }

        std::string inner = result.substr(start, end - start - 1);
        std::string resolved;

        // === RECURSIVE RESOLUTION ===
        inner = resolveVars(inner, self, other, global);

        // === VARIABLE ACCESS ===
        if (inner.rfind("self.", 0) == 0) {
            resolved = JSONHandler::Get::Any<std::string>(self, inner.substr(5), "0");
        } else if (inner.rfind("other.", 0) == 0) {
            resolved = JSONHandler::Get::Any<std::string>(other, inner.substr(6), "0");
        } else if (inner.rfind("global.", 0) == 0) {
            resolved = JSONHandler::Get::Any<std::string>(global, inner.substr(7), "0");
        } else {
            // === EXPRTK EVALUATION ===
            resolved = evaluateExpression(inner);
        }

        // Replace the $(...) with resolved value
        result.replace(pos, end - pos, resolved);

        // Restart search from current position
        pos += resolved.size();
    }
    return result;
}

