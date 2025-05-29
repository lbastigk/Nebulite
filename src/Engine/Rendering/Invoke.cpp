#include "Invoke.h"
#include "RenderObject.h"  // include full definition of RenderObject


Invoke::Invoke() {

}


bool Invoke::isTrue(std::shared_ptr<InvokeEntry> cmd, std::shared_ptr<RenderObject> otherObj, bool resolveEqual) {
    // Same send
    if((!resolveEqual) && (cmd->selfPtr.get() == otherObj.get())){
        return false;
    }

    // Resolve logical statement
    std::string logic = resolveVars(cmd->logicalArg, *cmd->selfPtr->getDoc(), *otherObj.get()->getDoc(), *global);
    if (evaluateExpression(logic) == 0.0){
        return false;
    }
    else{
        return true;
    }
}


// Checks an object against all linked invokes.
// True pairs are put into a special vector
void Invoke::checkAgainstList(std::shared_ptr<RenderObject> obj){
    for (auto& cmd : commands){
        if(isTrue(cmd,obj)){
            truePairs.push_back(std::make_pair(cmd,obj));
        }
    }
}

void Invoke::updateValueOfKey(std::string type, std::string key,std::string valStr, rapidjson::Document *doc){
    if        (type == "set")       JSONHandler::Set::Any<double>(*doc,key,std::stod(valStr));
    else if   (type == "setInt")    JSONHandler::Set::Any<int>(*doc,key,std::stod(valStr));
    else if   (type == "add")       JSONHandler::Set::Any<double>(*doc,key,JSONHandler::Get::Any<double>(*doc, key, 0.0) + std::stod(valStr));
    else if   (type == "multiply")  JSONHandler::Set::Any<double>(*doc,key,JSONHandler::Get::Any<double>(*doc, key, 0.0) * std::stod(valStr));
    else if   (type == "concat")    JSONHandler::Set::Any<std::string>(*doc,key,JSONHandler::Get::Any<std::string>(*doc, key, "") + valStr);
    else if   (type == "setStr")    JSONHandler::Set::Any<std::string>(*doc,key,valStr);
}

// Checks a given invoke cmd against objects in buffer
// as objects have constant pointers, using RenderObject& is possible
void Invoke::updateGlobal(std::shared_ptr<InvokeEntry> cmd, std::shared_ptr<RenderObject> otherObj) {
    // === SELF update ===
    for(auto InvokeTriple : cmd.get()->invokes_self){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *otherObj.get()->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === OTHER update ===
    for(auto InvokeTriple : cmd.get()->invokes_other){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *otherObj.get()->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === GLOBAL update ===
    for(auto InvokeTriple : cmd.get()->invokes_global){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *otherObj.get()->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === Functioncalls ===
    for(auto call : cmd.get()->functioncalls){
        // replace vars
        call = resolveVars(call,*cmd->selfPtr->getDoc(),*otherObj.get()->getDoc(), *global);

        // attach to task queue
        tasks->emplace_back(call);
    }
}

void Invoke::updateLocal(std::shared_ptr<InvokeEntry> cmd){
    // === SELF update ===
    for(auto InvokeTriple : cmd.get()->invokes_self){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *cmd->selfPtr->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === GLOBAL update ===
    for(auto InvokeTriple : cmd.get()->invokes_global){
        if (!InvokeTriple.key.empty()) {
            std::string valStr = resolveVars(InvokeTriple.value, *cmd->selfPtr->getDoc(), *cmd->selfPtr->getDoc(), *global);
            updateValueOfKey(InvokeTriple.changeType, InvokeTriple.key,valStr, cmd->selfPtr->getDoc());
        } 
    }

    // === Functioncalls ===
    for(auto call : cmd.get()->functioncalls){
        // replace vars
        call = resolveVars(call,*cmd->selfPtr->getDoc(),*cmd->selfPtr->getDoc(), *global);

        // attach to task queue
        tasks->emplace_back(call);
    }
}

void Invoke::clear(){
    commands.clear();
    commands.shrink_to_fit();

    nextCommands.clear();
    nextCommands.shrink_to_fit();

    truePairs.clear();
    truePairs.shrink_to_fit();
}

void Invoke::update(){
    // check general vals
    for (auto pair : truePairs){
        updateGlobal(pair.first, pair.second);
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

void Invoke::append(std::shared_ptr<InvokeEntry> toAppend){
    nextCommands.push_back(toAppend);
}

double Invoke::evaluateExpression(const std::string& expr) {
    return te_interp(expr.c_str(),0);
}

std::string Invoke::resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global) {
    // Variables
    std::string resolved, inner;
    std::string result = input;
    size_t start, end;
    int depth;

    // loop through string, find $(...) and replace with variables
    while (result.find("$(") != std::string::npos) {
        for(int i = 0; i < result.size()-1; i++){
            if(result[i] == '$' && result[i+1] == '('){
                start = i + 2;
                depth = 1;
                end = start;
                while (end < result.size() && depth > 0) {
                    if (result[end] == '(')         depth++;
                    else if (result[end] == ')')    depth--;
                    ++end;
                }
                if (depth != 0) {
                    // Unmatched parentheses
                    std::cerr << "No matching closing paranthesis found!";
                    break;
                }
                inner = result.substr(start, end - start - 1);

                // === VARIABLE ACCESS ===
                bool evaled_val = false;

                // check for reference to self
                if (inner.starts_with("self.")) {
                    evaled_val = true;
                    resolved = JSONHandler::Get::Any<std::string>(self, inner.substr(5), "0");
                } 
                // check for reference to other
                else if (inner.starts_with("other.")) {
                    evaled_val = true;
                    resolved = JSONHandler::Get::Any<std::string>(other, inner.substr(6), "0");
                } 
                // check for reference to global
                else if (inner.starts_with("global.")) {
                    evaled_val = true;
                    resolved = JSONHandler::Get::Any<std::string>(global, inner.substr(7), "0");
                } 
                // if none of these are true, it's an expression like $(1+1)
                // make sure there is no internal reference that still needs to be resolved
                else if (inner.find("$(") == std::string::npos){
                    evaled_val = true;
                    if(StringHandler::isNumber(inner)){
                        resolved = inner;
                    }
                    else{
                        resolved = std::to_string(evaluateExpression(inner));
                    }
                }
                
                // Replace only if it was evaluated
                if(evaled_val){
                    // Replace the $(...) with resolved value
                    result.replace(i, end - i, resolved);
                    i += result.size();
                }
            }
        }   
    }
    return result;
}


std::string Invoke::resolveGlobalVars(const std::string& input) {
    return resolveVars(input,emptyDoc,emptyDoc,*global);
}


