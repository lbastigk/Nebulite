#pragma once
#include <thread>


// NEBULITE INVOKE CLASS
/*

Invoke is used for interactions between objects currently in memory (on/near screen)
Since a 3x3 Display Resolution grid is loaded at each time, the headup
should be minimal as the usual resolution planned is about 16x16 objects on screen

Each object is able to send an invoke that is checked against each other invoke for an action. 

The following is provided to each invoke: 
- SELF
  used to manipulate itself
- OTHER
  used to manipulate the other object
  e.G. self is a solid block with the invoke, other is a moving object
  other.canMove.Left/Right... can be used to tell the object it cant move into the solid object
- GLOBAL
  count how many wolfes were killed
  keep track of quest stages: e.g. on pickup, send an invoke to modify quest stage
- FUNCTIONCALLS
  used to call functions in the maintree to manipulate the game
  e.g.: "echo Invoke Activated!"
  e.g.: "env-deload", "env-load ./Resources/Levels/stage_$($(global.currentStage)+1)"
- TOPIC
  Each Renderobject subscribes to certain topics. Each invoke itself represents one topic.
  For instance, it is possible to add the gravity invoke to an object OBJ1 but not subscribe to it,
  Meaning that other objects with subscription are attracted to OBJ1, but OBJ1 stays in place.
  Or one might only subscribe the player/npcs to the HitBox invokes, reducing the number of hitbox checks.


This also allows to store stuff for other objects to change that are currently not in memory
Like global.levelstate or similiar

*/


//-----------------------------------------
// THREADING SETTINGS

// Size of Batches
#define THREADED_MIN_BATCHSIZE 200  // Based on benchmark tests. Last check: 2025-07

// General Includes
#include <string>
#include <vector>
#include <deque>
#include <shared_mutex>

// Local
#include "tinyexpr.h"
#include "JSON.h"

// Keywords for resolving: $(1+1) , $(global.time.t) , ...
#define InvokeResolveKeyword '$'
#define InvokeResolveKeywordWithOpenParanthesis "$("
namespace Nebulite{

// Forward declaration of RenderObject
class RenderObject;
class Invoke{
public:
    //--------------------------------------------
    // Class-Specific Structures:


    // ==== EXPRESSION PARSING ====
    struct Node {
      // Each expression is pre-processed using a Tree build of Nodes
      // Each node is a part of the expression:
      // - Literal:     "this is a literal"
      // - Variable:    "$(global.time.t)"
      // - Mix_eval:    "$(1+$(global.time.t))"
      // - Mix_no_eval: "The time is: $(global.time.t)"
      //
      // Type Mix indicates Children!
      //
      // Example, say self.variable = 2
      // 
      // Version 1: Mix_eval
      // $($(self.variable) + 1)
      // Root has a $(...)
      // - children_1 : text = self.variable, type = Variable
      // - children_2 : text = " + 1",        type = Literal
      // Result: "3"
      //
      //
      // Version 1: Mix_no_eval
      // $(self.variable) + 1
      // Root has no $(...)
      // - children[0] : text = self.variable, type = Variable
      // - children[1] : text = " + 1",        type = Literal
      // Result: "2 + 1"
      enum class Type {
          Literal,      // Plain text
          Variable,     // $(self.value) or similar
          Mix_eval,     // An expression like $($(self.var) + 1), must evaluate entire subtree
          Mix_no_eval   // A mix of variables and literals, but not wrapped in $(...), just concatenate
      };
      Type type = Type::Literal;
      std::string text;
      std::vector<std::shared_ptr<Nebulite::Invoke::Node>> children; // for nested variables (if Expr)

      // Context on what data resource is taken from
      enum class ContextType { None, Self, Other, Global, Resources };
      ContextType context = ContextType::None;

      // Cast type. E.g.: $f(...) or $i(...)
      enum class CastType { None, Float, Int };
      CastType cast = CastType::None;

      // doc[key] being used
      std::string key;

      // If the node contains just a number: $(100)
      bool isNumericLiteral = false;

      // Making evaluation faster by checking if a parent would evaluate this node:
      // e.g.: $(1 + $(2 + 3))
      // makes sure that the inner $(2+3) expression is not resolved:
      // $(1 + 5.0000)  -> BAD, total calls for expr is 2
      // $(1 + (2 + 3)) -> BETTER. only one call
      bool insideEvalParent = false;      
    };

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
    
    // Parsed into:
    struct AssignmentExpression{
      enum class Operation {null, set,add,multiply,concat};
      Operation operation;                      // set, add, multiply, concat
      enum class Type {null, Self, Other, Global};
      Type onType;                              // Self, Other, Global, determines which doc is used
      std::string key;                          // e.g. "posX"
      std::string value;                        // e.g. "0", "$($(self.posX) + 1)"
      bool valueContainsReference = true;       // if value contains a reference keyword, e.g. "$(self.posX)" or "$(global.time.t)"
    };
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

    // ==== INVOKE PARSING HELPER FUNCTION ====  

    // Function to convert a JSON doc of Renderobject into a vector of Invoke::Entry
    static void parseFromJSON(
      Nebulite::JSON& doc, 
      std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_global, 
      std::vector<std::shared_ptr<Nebulite::Invoke::Entry>>& entries_local, 
      Nebulite::RenderObject* self
    );

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
    absl::flat_hash_map<std::string, std::shared_ptr<Nebulite::Invoke::Node>> exprTree;

    
    //----------------------------------------------------------------
    // Private functions

    // Runs all entries in an invoke with self and other given
    void updatePair(const std::shared_ptr<Nebulite::Invoke::Entry>& entries_self, Nebulite::RenderObject* Obj_other);

    // Evaluating expression with already replaced self/other/global etc. relations
    double evaluateExpression(const std::string& expr);

    // Resolving self/other/global references
    std::string resolveVars(const std::string& input, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global);
    
    // Main function for turning a string expression into a Node Tree
    std::shared_ptr<Nebulite::Invoke::Node> expressionToTree(const std::string& input);

    // turn nodes that hold just constant to evaluate into text
    // e.g. $(1+1) is turned into 2.000...
    void foldConstants(const std::shared_ptr<Nebulite::Invoke::Node>& node);

    // Helper funtion for evaluateNode for parsing 
    std::shared_ptr<Nebulite::Invoke::Node> parseNext(const std::string& input, size_t& i);

    // Take a pre-processed node and resolve all expressions and vars of this and nodes below
    //
    // Examples:
    // $($(global.constants.pi) + 1)  -> 4.141..
    //   $(global.constants.pi) + 1   -> 3.141... + 1
    // Time is: $(global.time.t)      -> Time is: 11.01
    std::string evaluateNode(const std::shared_ptr<Nebulite::Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent);

    // Helper function for accessing a variable from self/other/global/Resources
    std::string nodeVariableAccess(const std::shared_ptr<Nebulite::Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent);
};
}
