#pragma once

// TODO: Implement a subscription based invoke loading
// Each object subscribes to invokes and sends invokes itself
// Each object gets pointer to invokes in the global pool of its subscription
// When receiving invokes, the receiver is other
// 
// However, a more appropriate way might be to stack the effects on self and other
// Lots of thinking required here, maybe take a few day to rework architecture
// 
// Also, an approximation for invoke calls for a typical game is needed. 
// Perhaps implementing a horizontal slice soon is a good idea

/*

Invoke is used for interactions between objects currently in memory (on screen)
Since a 3x3 Display Resolution grid is loaded at each time, the headup
should be minimal as the usual resolution planned is about 16x16 objects on screen

Each object is able to send an invoke that is checked against each other invoke for an action. 

Example, this is how a gravity invoke might be realized. 

Part 1: Calculating acceleration
{
  "logicalArg": "$($(self.id) != $(other.id)) and $(other.physics.isGrav)",
  "isGlobal": true,
  "self_invokes": [
    {
      "changeType": "add",
      "key": "physics.aX",
      "value": "$($(global.G) * $(other.physics.mass) * ( $(other.posX) - $(self.posX)  ) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1)^(3/2) ))"
    },
    {
      "changeType": "add",
      "key": "physics.aY",
      "value": "$($(global.G) * $(other.physics.mass) * ( $(other.posY) - $(self.posY)  ) / ( ( ($(other.posX) - $(self.posX))^2 + ($(other.posY) - $(self.posY))^2 + 1)^(3/2) ))"
    }
  ],
  "other_invokes": [],
  "global_invokes": [],
  "functioncalls": []
}

Part 2: Acceleration to position integration
{
  "logicalArg": "1",
  "isGlobal": false,
  "self_invokes": [
    {
      "changeType": "add",
      "key": "physics.vX",
      "value": "$($(self.physics.aX) * $(global.dt))"
    },
    {
      "changeType": "add",
      "key": "physics.vY",
      "value": "$($(self.physics.aY) * $(global.dt))"
    },
    {
      "changeType": "add",
      "key": "posX",
      "value": "$($(self.physics.vX) * $(global.dt))"
    },
    {
      "changeType": "add",
      "key": "posY",
      "value": "$($(self.physics.vY) * $(global.dt))"
    },
    {
      "changeType": "set",
      "key": "physics.aX",
      "value": "0"
    },
    {
      "changeType": "set",
      "key": "physics.aY",
      "value": "0"
    }
  ],
  "other_invokes": [],
  "global_invokes": [],
  "functioncalls": []
}

The following is provided to each invoke: 
- self as rapidjson doc
  - used to manipulate itself
- other as rapidjson doc
  - used to manipulate the other object
  - e.G. self is a solid block with the invoke, other is a moving object
  - other.canMove.Left/Right... can be used to tell the object it cant move in the solid object
- global as rapidjson doc
  - count how many wolfes were killed
  - keep track of quest stages: e.g. on pickup, send an invoke to modify quest stage
- a tasklist to write new functioncalls into
  - e.g.: "echo Invoke Activated!"
  - e.g.: "env-deload", "env-load ./Resources/Levels/stage_$($(global.currentStage)+1)"


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
    std::string resolveGlobalVars(const std::string& input);

    void updateValueOfKey(std::string type, std::string key,std::string valStr, rapidjson::Document *doc);

    rapidjson::Document* getGlobalPointer(){return global;};
private:
    rapidjson::Document emptyDoc;
    rapidjson::Document* global = nullptr;
    std::vector<std::shared_ptr<InvokeEntry>> commands;
    std::vector<std::shared_ptr<InvokeEntry>> nextCommands; 
    std::vector<std::pair<std::shared_ptr<InvokeEntry>,std::shared_ptr<RenderObject>>> truePairs;

    // pointer to queue
    std::deque<std::string>* tasks = nullptr;    
};

