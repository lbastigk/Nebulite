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
#include <deque>
#include "tinyexpr.h"
#include "JSONHandler.h"



struct InvokeCommand{
    std::shared_ptr<RenderObject> selfPtr;  // store self
    std::string logicalArg;                 //e.g. $self.posX > $other.posY
    std::string selfChangeType;             //set, append, add, multiply etc.
    std::string selfKey;                    // what key to change in self
    std::string selfValue;                  // value
    std::string otherChangeType;
    std::string otherKey;
    std::string otherValue;
    std::string globalChangeType;
    std::string globalKey;
    std::string globalValue;
    std::string type = "continous";
};

/*

*/
struct InvokeTriple {
    std::string changeType;
    std::string key;
    std::string value;
};

struct InvokeEntry{
    std::shared_ptr<RenderObject> selfPtr;      // store self
    std::string logicalArg;                     //e.g. $self.posX > $other.posY
    std::vector<InvokeTriple> invokes_self;     // vector : key-value pair
    std::vector<InvokeTriple> invokes_other;
    std::vector<InvokeTriple> invokes_global;
    std::vector<std::string> functioncalls;     // function calls, e.g. load, save etc
    bool isGlobal = true;
};
// EXAMPLE:
/*
{
  "logicalArg": "$self.posX > $other.posY",
  "isGlobal": true,
  "self_invokes": [
    {
      "changeType": "set",
      "key": "posX",
      "value": "100"
    },
    {
      "changeType": "add",
      "key": "velocity",
      "value": "5"
    }
  ],
  "other_invokes": [
    {
      "changeType": "multiply",
      "key": "health",
      "value": "0.9"
    }
  ],
  "global_invokes": [
    {
      "changeType": "append",
      "key": "log",
      "value": "\"Action triggered\""
    }
  ],
  "functioncalls": [
    "load",
    "save"
  ]
}
*/

class Invoke{
public:
    // Setting up invoke by linking it to a global doc
    Invoke();
    void linkGlobal(rapidjson::Document& globalDocPtr){
        global = &globalDocPtr;
    }
    void linkQueue(std::deque<std::string>& queue){
        tasks = &queue;
    }

    void clear();
    
    // Append invoke command
    void append(std::shared_ptr<InvokeEntry> toAppend);


    void checkLoop();
    void checkGeneral();

    

    void checkAgainstList(std::shared_ptr<RenderObject> otherObj);
    bool isTrue(std::shared_ptr<InvokeEntry> cmd, std::shared_ptr<RenderObject> otherObj, bool resolveEqual=true);
    void update();
    void updateGlobal(std::shared_ptr<InvokeEntry> cmd, std::shared_ptr<RenderObject> otherObj);
    void updateLocal(std::shared_ptr<InvokeEntry> cmd);
    
    // Check against list
    
    
    // Get Invokes for next frame
    void getNewInvokes();
    

    // For evaluating sing expression
    double evaluateExpression(const std::string& expr);
    std::string resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global);


    void updateValueOfKey(std::string type, std::string key,std::string valStr, rapidjson::Document *doc);

    rapidjson::Document* getGlobalPointer(){return global;};
private:
    rapidjson::Document* global = nullptr;
    std::vector<std::shared_ptr<InvokeEntry>> commands;
    std::vector<std::shared_ptr<InvokeEntry>> nextCommands; 
    std::vector<std::pair<std::shared_ptr<InvokeEntry>,std::shared_ptr<RenderObject>>> truePairs;

    // pointer to queue
    std::deque<std::string>* tasks = nullptr;

    // exprtk stuff:
    //typedef exprtk::expression<double> expression_t;
    //typedef exprtk::parser<double> parser_t;
    //expression_t expression;
    //parser_t parser;

    
};

