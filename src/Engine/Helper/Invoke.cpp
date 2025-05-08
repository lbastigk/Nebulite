#include "Invoke.h"
#include "RenderObject.h"  // include full definition of RenderObject


Invoke::Invoke() {

}


bool Invoke::isTrue(std::shared_ptr<InvokeCommand> cmd, RenderObject& otherObj, bool resolveEqual) {
    // Same send
    if((!resolveEqual) && (cmd->selfPtr == &otherObj)){
        return false;
    }

    // Resolve logical statement
    std::string logic = resolveVars(cmd->logicalArg, *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
    if (evaluateExpression(logic) == 0.0){
        return false;
    }
    else{
        return true;
    }
}


// Checks an object against all linked invokes.
// True pairs are put into a special vector
void Invoke::checkAgainstList(RenderObject& obj){
    for (auto& cmd : commands){
        if(isTrue(cmd,obj)){
            truePairs.push_back(std::make_pair(cmd,&obj));
        }
    }
}

void Invoke::updateValueOfKey(std::string type, std::string key,std::string valStr, rapidjson::Document *doc){
    double val = evaluateExpression(valStr);
    double oldVal = JSONHandler::Get::Any<double>(*doc, key, 0.0);

    if        (type == "set")       JSONHandler::Set::Any<double>(*doc,key,val);
    else if   (type == "setInt")    JSONHandler::Set::Any<int>(*doc,key,val);
    else if   (type == "add")       JSONHandler::Set::Any<double>(*doc,key,oldVal + val);
    else if   (type == "multiply")  JSONHandler::Set::Any<double>(*doc,key,oldVal * val);
    else if   (type == "concat")    JSONHandler::Set::Any<std::string>(*doc,key,JSONHandler::Get::Any<std::string>(*doc, key, "") + valStr);
    else if   (type == "setStr")    JSONHandler::Set::Any<std::string>(*doc,key,valStr);
}

// Checks a given invoke cmd against objects in buffer
// as objects have constant pointers, using RenderObject& is possible
void Invoke::updatePair(std::shared_ptr<InvokeCommand> cmd, RenderObject& otherObj) {
    // === SELF update ===
    if (!cmd->selfKey.empty() && !cmd->selfChangeType.empty()) {
        std::string valStr = resolveVars(cmd->selfValue, *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
        updateValueOfKey(cmd->selfChangeType, cmd->selfKey,valStr, cmd->selfPtr->getDoc());
    }

    // === OTHER update ===
    if (!cmd->otherChangeType.empty() && !cmd->otherChangeType.empty()) {
        std::string valStr = resolveVars(cmd->otherValue,  *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
        updateValueOfKey(cmd->otherChangeType, cmd->otherKey,valStr, otherObj.getDoc());
    }

    // === GLOBAL update ===
    if (!cmd->globalKey.empty() && !cmd->globalChangeType.empty()) {
        std::string valStr = resolveVars(cmd->globalValue,  *cmd->selfPtr->getDoc(), *otherObj.getDoc(), *global);
        updateValueOfKey(cmd->globalChangeType, cmd->globalKey,valStr, global);
    }
}

void Invoke::update(){
    // check general vals
    for (auto pair : truePairs){
        updatePair(pair.first, *pair.second);
    }
    truePairs.clear();
    truePairs.shrink_to_fit();
}

// Called after a full renderer update to get all extracted invokes
void Invoke::getNewInvokes(){
    commands.clear();
    commands.shrink_to_fit();
    commands.swap(nextCommands);    // Swap in the new set of commands
}

void Invoke::append(std::shared_ptr<InvokeCommand> toAppend){
    nextCommands.push_back(toAppend);
}

double Invoke::evaluateExpression(const std::string& expr) {
    return te_interp(expr.c_str(),0);
}

std::string Invoke::resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global) {
    // Variables
    std::string resolved, inner;
    std::string result = input;
    size_t pos = 0;
    size_t start, end;
    int depth;

    // loop through string, find $(...) and replace with variables
    while ((pos = result.find("$(", pos)) != std::string::npos) {
        start = pos + 2;
        depth = 1;
        end = start;
        while (end < result.size() && depth > 0) {
            if (result[end] == '(')         depth++;
            else if (result[end] == ')')    depth--;
            ++end;
        }

        if (depth != 0) {
            // Unmatched parentheses
            break;
        }

        inner = result.substr(start, end - start - 1);
        

        // === RECURSIVE RESOLUTION ===
        //inner = resolveVars(inner, self, other, global);  // usually not needed...

        // === VARIABLE ACCESS ===
        if (inner.rfind("self.", 0) == 0) {
            resolved = JSONHandler::Get::Any<std::string>(self, inner.substr(5), "0");
        } else if (inner.rfind("other.", 0) == 0) {
            resolved = JSONHandler::Get::Any<std::string>(other, inner.substr(6), "0");
        } else if (inner.rfind("global.", 0) == 0) {
            resolved = JSONHandler::Get::Any<std::string>(global, inner.substr(7), "0");
        } else {
            // === EXPRTK EVALUATION ===
            //resolved = evaluateExpression(inner);  // usually not needed...
        }

        // Replace the $(...) with resolved value
        result.replace(pos, end - pos, resolved);

        // Restart search from current position
        pos += resolved.size();
    }
    return result;
}

