#include "Invoke.h"

Invoke::Invoke(rapidjson::Document& globalDocPtr) {
    global = &globalDocPtr;
}


void Invoke::check(RenderObject& selfObj, RenderObject& otherObj) {

    for (auto& cmd : commands) {
        std::string logic = resolveVars(cmd.logicalArg, *selfObj.getDoc(), *otherObj.getDoc(), *global);
        
        if (evaluateExpression(logic) == "0") continue;

        // === SELF update ===
        if (!cmd.selfKey.empty() && !cmd.selfChangeType.empty()) {
            std::string valStr = resolveVars(cmd.selfValue, *selfObj.getDoc(), *otherObj.getDoc(), *global);
            double val = std::stod(valStr);
            double oldVal = JSONHandler::Get::Any<double>(*selfObj.getDoc(), cmd.selfKey, 0.0);

            if (cmd.selfChangeType == "set") {
                selfObj.valueSet<double>(cmd.selfKey, val);
            } else if (cmd.selfChangeType == "add") {
                selfObj.valueSet<double>(cmd.selfKey, oldVal + val);
            } else if (cmd.selfChangeType == "multiply") {
                selfObj.valueSet<double>(cmd.selfKey, oldVal * val);
            } else if (cmd.selfChangeType == "append") {
                std::string oldStr = JSONHandler::Get::Any<std::string>(*selfObj.getDoc(), cmd.selfKey, "");
                selfObj.valueSet<std::string>(cmd.selfKey, oldStr + valStr);
            }
        }

        // === OTHER update ===
        if (!cmd.otherKey.empty() && !cmd.otherChangeType.empty()) {
            std::string valStr = resolveVars(cmd.otherValue, *selfObj.getDoc(), *otherObj.getDoc(), *global);
            double val = std::stod(valStr);
            double oldVal = JSONHandler::Get::Any<double>(*otherObj.getDoc(), cmd.otherKey, 0.0);

            if (cmd.otherChangeType == "set") {
                otherObj.valueSet<double>(cmd.otherKey, val);
            } else if (cmd.otherChangeType == "add") {
                otherObj.valueSet<double>(cmd.otherKey, oldVal + val);
            } else if (cmd.otherChangeType == "multiply") {
                otherObj.valueSet<double>(cmd.otherKey, oldVal * val);
            } else if (cmd.otherChangeType == "append") {
                std::string oldStr = JSONHandler::Get::Any<std::string>(*otherObj.getDoc(), cmd.otherKey, "");
                otherObj.valueSet<std::string>(cmd.otherKey, oldStr + valStr);
            }
        }

        // === GLOBAL update ===
        if (!cmd.globalKey.empty() && !cmd.globalChangeType.empty()) {
            std::string valStr = resolveVars(cmd.globalValue, *selfObj.getDoc(), *otherObj.getDoc(), *global);
            double val = std::stod(valStr);
            double oldVal = JSONHandler::Get::Any<double>(*global, cmd.globalKey, 0.0);

            if (cmd.globalChangeType == "set") {
                JSONHandler::Set::Any<double>(*global, cmd.globalKey, val);
            } else if (cmd.globalChangeType == "add") {
                JSONHandler::Set::Any<double>(*global, cmd.globalKey, oldVal + val);
            } else if (cmd.globalChangeType == "multiply") {
                JSONHandler::Set::Any<double>(*global, cmd.globalKey, oldVal * val);
            } else if (cmd.globalChangeType == "append") {
                std::string oldStr = JSONHandler::Get::Any<std::string>(*global, cmd.globalKey, "");
                JSONHandler::Set::Any<std::string>(*global, cmd.globalKey, oldStr + valStr);
            }
        }
    }
}

void Invoke::clear(){
    commands.clear();
}

void Invoke::append(InvokeCommand cmd){
    commands.push_back(cmd);
}

std::string Invoke::evaluateExpression(const std::string& expr) {
    typedef exprtk::expression<double> expression_t;
    typedef exprtk::parser<double> parser_t;

    exprtk::symbol_table<double> symbol_table;
    expression_t expression;
    expression.register_symbol_table(symbol_table);
    parser_t parser;

    double result = 0.0;
    if (parser.compile(expr, expression)) {
        result = expression.value();
    }

    return std::to_string(result);
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
