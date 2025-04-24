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

// Forward declaration of RenderObject
class RenderObject;

#include <string>
#include <vector>
#include "exprtk.hpp"
#include <JSONHandler.h>


struct InvokeCommand{
    RenderObject *selfPtr;          // store self
    std::string logicalArg;         //e.g. $self.posX > $other.posY
    std::string selfChangeType;     //set, append, add, multiply etc.
    std::string selfKey;            // what key to change in self
    std::string selfValue;          // value
    std::string otherChangeType;
    std::string otherKey;
    std::string otherValue;
    std::string globalChangeType;
    std::string globalKey;
    std::string globalValue;
    std::string type = "continous";
};

class Invoke{
public:
    // Setting up invoke by linking it to a global doc
    Invoke(rapidjson::Document& globalDocPtr);
    
    // Append invoke command
    void append(std::shared_ptr<InvokeCommand> toAppend);

    // Check Renderobject against invokes, modify
    void check(std::shared_ptr<InvokeCommand> cmd, RenderObject& otherObj); 
    void checkLoop();
    
    // Check against list
    void checkAgainstList(RenderObject& obj);
    
    // Get Invokes for next frame
    void getNewInvokes();
    

    // For evaluating sing expression
    double evaluateExpression(const std::string& expr);
    std::string resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global);

private:
    rapidjson::Document* global = nullptr;
    std::vector<std::shared_ptr<InvokeCommand>> commands;
    std::vector<std::shared_ptr<InvokeCommand>> nextCommands; 

    std::vector<std::shared_ptr<InvokeCommand>> loopCommands;
    std::vector<std::shared_ptr<InvokeCommand>> nextLoopCommands;

    // exprtk stuff:
    typedef exprtk::expression<double> expression_t;
    typedef exprtk::parser<double> parser_t;

    exprtk::symbol_table<double> symbol_table;
    expression_t expression;
    parser_t parser;
};

