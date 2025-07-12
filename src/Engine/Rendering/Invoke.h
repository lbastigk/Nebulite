#pragma once

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
//  88888888888 .d88888b.  8888888b.   .d88888b.  
//      888    d88P" "Y88b 888  "Y88b d88P" "Y88b 
//      888    888     888 888    888 888     888 
//      888    888     888 888    888 888     888 
//      888    888     888 888    888 888     888 
//      888    888     888 888    888 888     888 
//      888    Y88b. .d88P 888  .d88P Y88b. .d88P 
//      888     "Y88888P"  8888888P"   "Y88888P"  
//    
//--------------------------------------------------
// TODO: 
// - threadable local invokes
//   Inside Environment/ROC, reinsert batches for threading
//   but small batchsize, 10 perhaps?
// - local functioncalls for Renderobject:
//   each Renderobject with a tree to functions like:
//   - Position-text
//   - addChildren
//   - removeChildren
//   - reloadInvokes
//   - addInvoke
//   - removeAllInvokes
//--------------------------------------------------

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
    

    // Entry describing:
    // doc[key] = f(key,docs)
    // e.g.:
    // doc["var"] = doc["var"] * ( global[inner.otherVar] + 1)
    // is:
    // { 
    //    changeType : "multiply"
    //    key : "var"
    //    value : "$( $(global.inner.otherVar) + 1 )"
    // }
    #define USE_EVAL_STR 0
    struct InvokeTriple {
        enum class ChangeType {set,add,multiply,concat};
        ChangeType changeType;
        std::string key;
        std::string value;
        bool valueContainsResolveKeyword = true;
    };

    // Full entry consisting of:
    // - topic
    // - who self is
    // - logical argument (if-condition)
    // - invoke triples to self
    // - invoke triples to other
    // - invoke triples to global
    // - if invoke is global (broadcast topic)
    struct InvokeEntry{
        std::string topic = "all";
        Nebulite::RenderObject* selfPtr;      // store self
        std::string logicalArg;                     // e.g. $($(self.posX) > $(other.posY))
        std::vector<InvokeTriple> invokes_self;     // vector : key-value pair
        std::vector<InvokeTriple> invokes_other;
        std::vector<InvokeTriple> invokes_global;
        std::vector<std::string> functioncalls;     // function calls, e.g. load, save etc
        bool isGlobal = true;
    };

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
    void broadcast(const std::shared_ptr<Nebulite::Invoke::InvokeEntry>&);

    // Listen to a topic
    // Checks an object against all available invokes to a topic.
    // True pairs are put into a vector for later evaluation
    void listen(Nebulite::RenderObject* obj,std::string topic);

    //--------------------------------------------
    // Value checks

    // Check if cmd is true compared to other object
    bool isTrueGlobal(const std::shared_ptr<Nebulite::Invoke::InvokeEntry>& cmd, Nebulite::RenderObject* otherObj);

    // Check if local invoke is true
    // Same as isTrueGlobal, but using self for linkage to other
    // Might be helpful to use an empty doc here to supress any value from other being true
    bool isTrueLocal(const std::shared_ptr<Nebulite::Invoke::InvokeEntry>& cmd);


    //--------------------------------------------
    // Updating

    // Updating self-other-pairs of invokes
    void update();
    
    // Same as updateGlobal, but without an other-object
    // Self is used as reference to other.
    void updateLocal(const std::shared_ptr<Nebulite::Invoke::InvokeEntry>& cmd_self);

    // Sets new value
    // Call representing functions of ChangeType in order to safely modify the document
    void updateValueOfKey(
      Nebulite::Invoke::InvokeTriple::ChangeType type, 
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

    // Mutex lock for tasks
    mutable std::recursive_mutex tasks_lock;

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
    // cmds["topic"][]
    absl::flat_hash_map<
      std::string, 
      std::vector<
        std::shared_ptr<Nebulite::Invoke::InvokeEntry>
      >
    > globalcommands;

    absl::flat_hash_map<
      std::string, 
      std::vector<
        std::shared_ptr<Nebulite::Invoke::InvokeEntry>
      >
    > globalcommandsBuffer; 

    // All pairs of last listen
    std::vector<
      std::vector<
        std::pair<
          std::shared_ptr<Nebulite::Invoke::InvokeEntry>,
          Nebulite::RenderObject*
        >
      >
    > pairs_threadsafe;

    // Map for each Tree
    std::shared_mutex exprTreeMutex;
    absl::flat_hash_map<std::string, std::shared_ptr<Nebulite::Invoke::Node>> exprTree;

    
    //----------------------------------------------------------------
    // Private functions

    void updateVectorOfInvokeTriples(std::vector<Nebulite::Invoke::InvokeTriple> *vectorInvokeTriples, JSON *self, JSON *other, JSON *global, JSON *docToManipulate);

    // Runs all entries in an invoke with self and other given
    void updatePair(const std::shared_ptr<Nebulite::Invoke::InvokeEntry>& cmd_self, Nebulite::RenderObject* Obj_other);

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
