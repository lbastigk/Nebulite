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

#include <Renderer.h>

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
public:
    // Setting up invoke by linking it to a global doc
    Invoke(rapidjson::Document& globalDocPtr);
    
    // Append invoke command
    void append(InvokeCommand toAppend);

    // Check Renderobject against invokes, modify
    // For now, perhaps checking each object against another? so N! many checks...
    // later on, using IDs to check "up" and "down":
    // - check1(other)
    // - check2(self)
    // -> Meaning, self does invoke, object info is stored
    // if an other is found on next renderer update, manipulate other first, keep track that self is to be changed
    // at the end of the renderer update, go through all self changes, update them too
    // IDs are used to determine who self is (perhaps using pos and an additional id?)
    // If two obj have same id and pos, update id
    // But this is only for later, for now lets just manually check N*(N-1) outside of class
    // or, storing pointer to self??
    void check(RenderObject& selfObj, RenderObject& otherObj);    
    

    // Clear all invokes (should be called each frame)
    void clear();

    static std::string evaluateExpression(const std::string& expr);
    static std::string resolveVars(const std::string& input, rapidjson::Document& self, rapidjson::Document& other, rapidjson::Document& global);

private:
    rapidjson::Document* global = nullptr;
    std::vector<InvokeCommand> commands;    // each update, do all commands and then delete them

    // make it 2 vectors? One for only self-manipulation since this one needs no overhead
    
};

