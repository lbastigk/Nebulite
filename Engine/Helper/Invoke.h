#pragma once

/*

Invoke is used for interactions between objects currently in memory (on screen)
Since a 3x3 Display Resolution grid is loaded at each time, the headup
should be minimal as the usual resolution planned is about 16x16 objects on screen

Each object is able to send an invoke that is checked against each other invoke for an action. 

Example: 

Using invoke, each solid object can send a request for any moving object:  

logicalArg: $( $(other.moves) && $( $($self.X - $other.X) < $($other.closestObjectRight)) && $($($self.X - $other.X) > 0))
otherChangeType:    set
otherKey:           closestObjectX
otherValue:         $($(self.X) - $(other.X))

Then an invoke can be send for each key press on keyRight:  

logicalArg: $other.isPlayer && $other.closesObjectRight > 15 && !$other.Moving
otherChangeType:    append
otherKey:           moveRuleSet
otherValue:         {ruleset for moving right}

The following is provided to each invoke: 
- self as rapidjson doc
- other as rapidjson doc
- global as rapidjson doc

Global enables other powerful settings such as:  
- count how many wolfes were killed
- keep track of quest stages: e.g. on pickup, send an invoke to modify quest stage

This also allows to store stuff for other objects to change that are currently not in memory
*/


#include <string>
#include <vector>
#include "exprtk.hpp"

#include <JSONHandler.h>

// Resolving Vars inside string recursively
// Find first $( and fitting ) , meaning same depth
// Call function on everything between
// if no $( exists, no more resolving, return same string
// else, check if it starts with self, other or global
// if it starts with $( but has no self,other,global at the start and no other $() inside, evaluate with exprtk
//
// e.g.:
// $(global.test) + 1  -> "test + 1" as the + 1 is not inside a $()
// $($(global.pi) + 1) -> "4.14159..."
// 
// Always return as string. If the answer is a logical expression, return 0 or 1
//
// Using JSONHandler:
//
// JSONHandler::Get::Any<std::string>(rapidjson::Document& doc, const std::string& fullKey, const T& defaultValue = T());
// full key for global.test1.test2 would be: "test1.test2"
// A default value of "0" is used
//

std::string evaluateExpression(const std::string& expr) {
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

std::string resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global) {
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

struct InvokeCommand{
    std::string selfID;         // since each invoke lasts one tick, perhaps location + num would be an idea?
    std::string logicalArg;     //e.g. $self.posX > $other.posY
    std::string selfChangeType; //set, append, add, multiply etc.
    std::string selfKey;        // what key to change in self
    std::string selfValue;      // value
    std::string otherChangeType;
    std::string otherKey;
    std::string otherValue;
    std::string globalChangeType; //set, append etc.
    std::string globalKey;        // what key to change in self
    std::string globalValue;      // value
};

class Invoke{
    Invoke(rapidjson::Document& global);
    

private:

    std::vector<InvokeCommand> commands;    // each update, do all commands and then delete them
};

