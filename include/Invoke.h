#pragma once

// NEBULITE INVOKE CLASS
/*
===========================================================
Invoke – Dynamic Object Logic Engine
===========================================================

PURPOSE:
The Invoke class is the maintainer of object logic in Nebulite, enabling
dynamic game behavior through JSON-defined rules separated from the codebase.

CORE PHILOSOPHY:
- EXPRESSIONS: Hot-swappable logic for simple, mathematical operations
- FUNCTION CALLS: Compiled logic for complex operations

DATA-DRIVEN DESIGN:
All object behavior is defined through JSON files loaded by RenderObjects,
allowing designers and modders to create complex game logic without
touching C++ code.

===========================================================
CONTEXT SYSTEM: Self-Other-Global
===========================================================

SELF-OTHER-GLOBAL MODEL:
Objects interact through a three-tier context system:

• SELF:   The object broadcasting the invoke
• OTHER:  The object listening for invokes 
• GLOBAL: Shared game state (time, input, settings)

SELF-GLOBAL CONTEXT (Local Operations):
- Object modifies itself based on global state
- Examples: Health regeneration, input response, timers
- JSON: "topic": "" (empty topic = local only)

SELF-OTHER-GLOBAL CONTEXT (Interactive Operations):  
- Objects interact with each other conditionally
- Examples: Collision, line-of-sight, proximity triggers
- JSON: "topic": "collision" (named topic = broadcast)

===========================================================
EXPRESSION SYSTEM: Hot-Swappable Logic
===========================================================

SINGLE-KEY MODIFICATIONS:
Expressions enable simple, direct variable changes without recompilation:

• Mathematical: "self.health += $(other.damage)"
• Conditional: "$(self.x) > 100 && self.health > 0"  
• Type casting: "$f($(self.score))" or "$i($(global.time))"
• Concatenation: "self.status |= ' - damaged'"

WIDE APPLICATION RANGE:
- Movement systems
- Health/damage calculations  
- Animation triggers
- State changes
- Resource management

RUNTIME FLEXIBILITY:
Change game behavior by editing JSON files - no binary recompilation needed.
Perfect for:
- Game balancing
- Rapid prototyping  
- Modding support
- Designer empowerment
- Debugging

===========================================================
FUNCTION CALL SYSTEM: Complex Logic
===========================================================

COMPILED OPERATIONS:
Function calls handle complex logic that expressions cannot.
Example functioncalls that maintainers might wish to implement:
• GLOBAL SCOPE: "functioncalls_global": ["spawn <link>", "save-game"]
• SELF SCOPE:   "functioncalls_self": ["play-animation", "flag-delete"]  
• OTHER SCOPE:  "functioncalls_other": ["reload-texture"]

WHEN TO USE FUNCTION CALLS:
- Multi-step algorithms
- File I/O operations
- Complex state machines
- Performance-critical operations
- External system integration
- Usage of objects not defined in JSON
- Moving/Copying JSON Sub-objects

BINARY RECOMPILATION REQUIRED:
Adding new function calls requires C++ implementation and recompilation,
but provides maximum performance and flexibility.

===========================================================
ARCHITECTURE BENEFITS
===========================================================

DESIGNER-FRIENDLY:
- JSON-based configuration
- No programming knowledge required
- Hot-reload capabilities
- Visual feedback through expressions

PERFORMANCE OPTIMIZED:
- Pre-parsed expression trees
- Constant folding optimization  
- Threaded batch processing
- Smart caching system

MODULAR DESIGN:
- Clean separation: expressions vs functions
- Topic-based broadcasting system
- Context-aware variable resolution
- Extensible function library

===========================================================
EXAMPLE WORKFLOW
===========================================================

1. DESIGN PHASE (JSON):
   Define object behavior through Diagrams, Flowcharts, and Pseudocode.
   Transform these into expressions and function calls.

2. RUNTIME PHASE (Engine):
   - Objects broadcast invokes to topics
   - Listeners evaluate logical conditions of topics they are subscribed to
   - Matching pairs execute expressions/functioncalls

3. ITERATION PHASE (Hot-reload):
   Modify JSON files, reload the engine, and see immediate results

This system bridges the gap between designer creativity and 
programmer control, enabling rapid iteration while maintaining 
high performance for complex operations.

===========================================================
DESIGN ENCOURAGEMENTS
===========================================================
- When designing a specific system, consider if it can be expressed as a series of expressions
- Define a system name that can be used as a topic for invokes
- When assigning values, prefer a nested structure, perhaps with that system name as the first key
- Avoid the "all" topic as much as possible, as it can lead to performance issues
- For unknown other-candidates, consider using an ambassador JSON object:
  - spawn ambassador
  - ambassador listens to all invokes or a specific topic if possible
  - finds relevant objects within the system and modifies them
  - example: objects at a certain position need to be found and modified. 
    Spawn a pre-defined ambassador at that position with an invoke that modifies objects:
    "logicalArg" : [ "eq($(self.posX),$(other.posX))", "eq($(self.posY),$(other.posY))" ]
    "exprs" : [ "other.health -= 10" ]
  - that ambassador might be visible: bullet, particle, selection square
    or invisible: pathfinding object 
*/


//-----------------------------------------
// THREADING SETTINGS

// Size of Batches
#define THREADED_MIN_BATCHSIZE 200  // Based on benchmark tests. Last check: 2025-07
//#define THREADED_MIN_BATCHSIZE 8000  // Making sure only one thread is used for tests

// General Includes
#include <string>
#include <vector>
#include <deque>
#include <shared_mutex>
#include <thread>

// Local
#include "tinyexpr.h"
#include "JSON.h"
#include "InvokeNode.h"
#include "DocumentCache.h"


namespace Nebulite{

// Forward declarations
class RenderObject;
class InvokeNode;

class Invoke{
public:
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
      "functioncalls_self": [],   // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.json"
      "functioncalls_other": []   // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.json"
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
    // myInvoke.json|push-back overwrites 'key1 -> value1'
    // Example JSON:
    /*
    "overwrites": {
        "key1 -> value1",
        "key2 -> value2"
    }
    */
    // This makes subkey-overwrites easier to parse, e.g.: "overwrites" [ "physics.G -> 9.81" ] 
    // turns an "$(overwrites.physics.G)" into "9.81" and 
    // defaults to "$(global.physics.G)" if not overwritten.
    
    //---------------------------------------------
    // Invoke epxressions are parsed into specific structs:
    struct AssignmentExpression{
      enum class Operation {null, set,add,multiply,concat};
      Operation operation;                      // set, add, multiply, concat
      enum class Type {null, Self, Other, Global};
      Type onType;                              // Self, Other, Global, determines which doc is used
      std::string key;                          // e.g. "posX"
      std::string value;                        // e.g. "0", "$($(self.posX) + 1)"
      bool valueContainsReference = true;       // if value contains a reference keyword, e.g. "$(self.posX)" or "$(global.time.t)"
    };
    //---------------------------------------------
    // ... Inside Entries:
    struct Entry{
      std::string topic = "all";                                  // e.g. "gravity", "hitbox", "collision"
      std::string logicalArg;                                     // e.g. "$(self.posX) > $(other.posY)"
      std::vector<Nebulite::Invoke::AssignmentExpression> exprs;  // vector of exprs
      std::vector<std::string> functioncalls_global;              // vector of function calls, e.g. "echo example"
      std::vector<std::string> functioncalls_self;                // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.json"
      std::vector<std::string> functioncalls_other;               // vector of function calls, e.g. "add_invoke ./Resources/Invokes/gravity.json"
      bool isGlobal = true;                                       // if true, the invoke is global and can be broadcasted to other objects: Same as a nonempty topic
      Nebulite::RenderObject* selfPtr;                            // store self
    };

    
    // Function to convert a JSON doc of Renderobject into a vector of Invoke::Entry


    //--------------------------------------------
    // General
    Invoke();

    // Setting up invoke by linking it to a global doc
    void linkGlobal(Nebulite::JSON& globalDocPtr){global = &globalDocPtr;}
    
    // Linking invoke to global queue for function calls
    void linkQueue(std::deque<std::string>& queue){tasks = &queue;}

    // Clearing all entries
    void clear();

    //--------------------------------------------
    // Getting

    // Global doc pointer
    Nebulite::JSON* getGlobalPointer(){return global;};

    // Global queue
    std::deque<std::string>* getQueue(){return tasks;};
    
    //--------------------------------------------
    // Send/Listen

    // Broadcast an invoke to other renderobjects to listen
    // Comparable to a radio, broadcasting on certain frequency determined by the string topic:
    void broadcast(const std::shared_ptr<Nebulite::Invoke::Entry>&);

    // Listen to a topic
    // Checks an object against all available invokes to a topic.
    // True pairs are put into a vector for later evaluation
    void listen(Nebulite::RenderObject* obj,std::string topic);

    //--------------------------------------------
    // Value checks

    // Check if cmd is true compared to other object
    bool isTrueGlobal(const std::shared_ptr<Nebulite::Invoke::Entry>& entry, Nebulite::RenderObject* otherObj);

    // Check if local invoke is true
    // Same as isTrueGlobal, but using self for linkage to other
    // Might be helpful to use an empty doc here to supress any value from other being true
    bool isTrueLocal(const std::shared_ptr<Nebulite::Invoke::Entry>& entry);


    //--------------------------------------------
    // Updating

    // Updating self-other-pairs of invokes
    void update();
    
    // Same as updateGlobal, but without an other-object
    // Self is used as reference to other.
    void updateLocal(const std::shared_ptr<Nebulite::Invoke::Entry>& entries_self);

    // Sets new value
    // Call representing functions of ChangeType in order to safely modify the document
    void updateValueOfKey(
      Nebulite::Invoke::AssignmentExpression::Operation operation, 
      const std::string& key, 
      const std::string& valStr, 
      Nebulite::JSON *doc
    );

    // same as resolveVars, but only using global variables. Self and other are linked to empty docs
    std::string resolveGlobalVars(const std::string& input);

private:
    //----------------------------------------------------------------
    // General Variables

    // Documents
    Nebulite::DocumentCache docCache;
    Nebulite::JSON emptyDoc;          // Linking an empty doc is needed for some functions
    Nebulite::JSON* global = nullptr; // Linkage to global doc

    // pointer to queue
    std::deque<std::string>* tasks = nullptr; 

    // Mutex lock for tasks and buffers
    mutable std::recursive_mutex tasks_lock;
    std::mutex entries_global_next_Mutex;
    std::mutex entries_global_Mutex;
    std::mutex pairsMutex;

    //----------------------------------------------------------------
    // TinyExpr

    // Custom TinyExpr functions
    class expr_custom{
    public:
        static double gt(double a, double b) {return a > b;}
        static double lt(double a, double b) {return a < b;}
        static double geq(double a, double b) {return a >= b;}
        static double leq(double a, double b) {return a <= b;}
        
        static double eq(double a, double b){return a == b;}
        static double neq(double a, double b){return a != b;}

        static double logical_and(double a, double b){return a && b;}
        static double logical_or(double a, double b){return a || b;}
        static double logical_not(double a){return !a;}

        static double sgn(double a){return 1.0 - 2.0*std::signbit(a);}
    };
    //absl::flat_hash_map<std::string, te_expr*> expr_cache;
    std::vector<te_variable> vars;


    //----------------------------------------------------------------
    // Hashmaps and vectors

    // Current and next commands
    absl::flat_hash_map<
      std::string, 
      std::vector<
        std::shared_ptr<Nebulite::Invoke::Entry>
      >
    > entries_global;

    absl::flat_hash_map<
      std::string, 
      std::vector<
        std::shared_ptr<Nebulite::Invoke::Entry>
      >
    > entries_global_next; 

    // All pairs of last listen
    std::vector<
      std::vector<
        std::pair<
          std::shared_ptr<Nebulite::Invoke::Entry>,
          Nebulite::RenderObject*
        >
      >
    > pairs_threadsafe;

    // Map for each Tree
    std::shared_mutex exprTreeMutex;
    absl::flat_hash_map<std::string, std::shared_ptr<Nebulite::InvokeNode>> exprTree;

    
    //----------------------------------------------------------------
    // Private functions

    // Runs all entries in an invoke with self and other given
    void updatePair(const std::shared_ptr<Nebulite::Invoke::Entry>& entries_self, Nebulite::RenderObject* Obj_other);

    // Evaluating expression with already replaced self/other/global etc. relations
    double evaluateExpression(const std::string& expr);

    // Resolving self/other/global references
    std::string resolveVars(const std::string& input, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global);

    friend class InvokeNodeHelper; // Allow InvokeNodeHelper to access private members
    InvokeNodeHelper nodeHelper;
};
}
