#pragma once
#include "InvokeExpressionPool.h"

//--------------------------------------------
// Class-Specific Structures:

// ==== INVOKE ENTRY PARSING ====
// Prases the JSON doc part "invokes" of a renderobject into Structures

// Example JSON:
/*
{
    "topic" : "...",      // e.g. "gravity", "hitbox", "collision". Empty topic for local invokes: no 'other', only 'self' and 'global'}
    "logicalArg": "...",  // e.g. "$(self.posX) > $(other.posY)
    "exprs" : [
        // all exprs in one vector
        // with the following structure:
        // type.key1.key2.... <assignment-operator> value
        // operators are: 
        // - set: =
        // - add: +=
        // - multiply: *=
        // - concat: |=
        "self.key1 = 0",
        "other.key2 *= 2",
        "global.key3 = 1"
    ],
    "functioncalls_global": [], // vector of function calls, e.g. "echo example"
    "functioncalls_self": [],   // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.jsonc"
    "functioncalls_other": []   // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.jsonc"
    }
*/
// TODO: Idea for Invoke ruleset overwrites:
// In addition, add the field "overwrites" to the JSON doc:
// "overwrites": {
//     "key1": "value1",
//     "key2": "value2"
// }
// Then, on parsing, the overwrites are applied:
// $(overwrites.key1) would be replaced by "value1"
// If, however, an overwrite is not found:
// $(overwrites.key3) would be replaced by $(global.key3)
// This allows us to flexibly overwrite values in the invoke without changing the original JSON file.
// Also, the behavior is well-defined, as it defaults to the global value if no overwrite is defined.
// Note: retrieval of overwrites in a type object might be difficult. Instead, perhaps:
// myInvoke.jsonc|push-back overwrites 'key1 -> value1'
// Example JSON:
/*
"overwrites": {
    "key1 -> value1",
    "key2 -> value2"
}
*/
// This makes subkey-overwrites easier to parse, e.g.: "overwrites" [ "physics.G -> 9.81" ] 
// turns an "$(overwrites.physics.G)" into "9.81" and 
// defaults to "{global.physics.G}" if not overwritten.

//---------------------------------------------
// Invoke epxressions are parsed into specific structs:
namespace Nebulite {

// Forward declarations
class RenderObject;

class InvokeAssignmentExpression{
public:
    enum class Operation {null, set,add,multiply,concat};
    Operation operation = Operation::null;                      // set, add, multiply, concat
    enum class Type {null, Self, Other, Global};
    Type onType = Type::null;                              // Self, Other, Global, determines which doc is used
    std::string key;                          // e.g. "posX"
    std::string value;                        // e.g. "0", "$($(self.posX) + 1)"
    Nebulite::InvokeExpressionPool expression;    // The parsed expression
    bool valueContainsReference = true;       // if value contains a reference keyword, e.g. "$(self.posX)" or "{global.time.t}"

    // Disable copy constructor and assignment
    InvokeAssignmentExpression(const InvokeAssignmentExpression&) = delete;
    InvokeAssignmentExpression& operator=(const InvokeAssignmentExpression&) = delete;

    // Enable move constructor and assignment
    InvokeAssignmentExpression() = default;
    InvokeAssignmentExpression(InvokeAssignmentExpression&& other) noexcept
        : operation(other.operation)
        , onType(other.onType)
        , key(std::move(other.key))
        , value(std::move(other.value))
        , expression(std::move(other.expression))
        , valueContainsReference(other.valueContainsReference)
    {
    }

    InvokeAssignmentExpression& operator=(InvokeAssignmentExpression&& other) noexcept {
        if (this != &other) {
            operation = other.operation;
            onType = other.onType;
            key = std::move(other.key);
            value = std::move(other.value);
            expression = std::move(other.expression);
            valueContainsReference = other.valueContainsReference;
        }
        return *this;
    }
};

//---------------------------------------------
// Each Renderobject holds its own InvokeEntries:
class InvokeEntry{
public:
    std::string topic = "all";                                      // e.g. "gravity", "hitbox", "collision"
    Nebulite::InvokeExpressionPool logicalArg;                          // e.g. "$(self.posX) > $(other.posY)"
    std::vector<Nebulite::InvokeExpressionPool> functioncalls_global;   // vector of function calls, e.g. "echo example"
    std::vector<Nebulite::InvokeExpressionPool> functioncalls_self;     // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.jsonc"
    std::vector<Nebulite::InvokeExpressionPool> functioncalls_other;    // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.jsonc"
    bool isGlobal = true;                                           // if true, the invoke is global and can be broadcasted to other objects: Same as a nonempty topic
    Nebulite::RenderObject* selfPtr = nullptr;                      // store self

    // Expressions
    std::vector<Nebulite::InvokeAssignmentExpression> exprs;

    // Make Entry non-copyable and non-movable
    InvokeEntry() = default;
    InvokeEntry(const InvokeEntry&) = delete;
    InvokeEntry& operator=(const InvokeEntry&) = delete;
    InvokeEntry(InvokeEntry&&) = delete;
    InvokeEntry& operator=(InvokeEntry&&) = delete;
};
}