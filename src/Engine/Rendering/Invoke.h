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

// Using threaded invokes or not
#define THREADED_INVOKE_EVAL 1

// Size of Batches/if to use
// 0 - no min size
// Best practice seems to be either 0 or 100 ?
/*

64 Object Gravity benchmark, rendering 600 Frames:
./build.sh ; cd ./Application ; /usr/bin/time -v ./bin/Nebulite task TaskFiles/Benchmarks/gravity.txt 2>&1 | grep Elapsed


0000 - 2.44s
0010 - 8.37s
0020 - 4.68s
0050 - 2.74s
0100 - 2.38s
0200 - 2.49s
0500 - 3.16s
1000 - 4.83s

*/
#define THREADED_MIN_BATCHSIZE 100

// Forward declaration of RenderObject
class RenderObject;

// General Includes
#include <string>
#include <vector>
#include <deque>
#include <shared_mutex>

// Local
#include "tinyexpr.h"
#include "JSON.h"

namespace Nebulite{
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
      std::vector<std::shared_ptr<Invoke::Node>> children; // for nested variables (if Expr)

      enum class ContextType { None, Self, Other, Global, Resources };
      ContextType context = ContextType::None;

      enum class CastType { None, Float, Int };
      CastType cast = CastType::None;

      std::string key;
      bool isNumericLiteral = false;
      bool insideEvalParent = false;
    };
    struct InvokeTriple {
        enum class ChangeType {set,add,multiply,concat};
        ChangeType changeType;
        std::string key;
        std::string value;
        
    };
    struct InvokeEntry{
        std::string topic = "all";
        std::shared_ptr<RenderObject> selfPtr;      // store self
        std::string logicalArg;                     // e.g. $($(self.posX) > $(other.posY))
        std::vector<InvokeTriple> invokes_self;     // vector : key-value pair
        std::vector<InvokeTriple> invokes_other;
        std::vector<InvokeTriple> invokes_global;
        std::vector<std::string> functioncalls;     // function calls, e.g. load, save etc
        bool isGlobal = true;

        enum class ThreadSafeType {None,Self,Other};
        ThreadSafeType threadSafeType = ThreadSafeType::None; //
    };

    //--------------------------------------------
    // General

    Invoke();

    // Setting up invoke by linking it to a global doc
    void linkGlobal(Nebulite::JSON& globalDocPtr){
        global = &globalDocPtr;
    }
    
    // Linking invoke to global queue for function calls
    void linkQueue(std::deque<std::string>& queue){
        tasks = &queue;
    }

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

    // Broadcast invoke
    void broadcast(const std::shared_ptr<InvokeEntry>& toAppend);

    // Listen to a topic
    // Checks an object against all available invokes to a topic.
    // True pairs are put into a vector for later evaluation
    void listen(const std::shared_ptr<RenderObject>& obj,std::string topic);

    //--------------------------------------------
    // Value checks

    // Check if cmd is true compared to other object
    bool isTrueGlobal(const std::shared_ptr<InvokeEntry>& cmd, const std::shared_ptr<RenderObject>& otherObj);

    // Check if local invoke is true
    bool isTrueLocal (const std::shared_ptr<InvokeEntry>& cmd);


    //--------------------------------------------
    // Updating

    void updatePairs();
    void updateGlobal(const std::shared_ptr<InvokeEntry>& cmd_self, const std::shared_ptr<RenderObject>& Obj_other);
    void updateLocal(const std::shared_ptr<InvokeEntry>& cmd_self);
    
    // Get Invokes for next frame
    // Empties current commands, shrinks and swaps with new commands vector.
    void getNewInvokes();

    // Sets new value
    void updateValueOfKey(
      Nebulite::Invoke::InvokeTriple::ChangeType type, 
      const std::string& key, 
      const std::string& valStr, 
      Nebulite::JSON *doc
    );

    // Resolving global references only in a string
    std::string resolveGlobalVars(const std::string& input);

private:
    //----------------------------------------------------------------
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
    };
    absl::flat_hash_map<std::string, te_expr*> expr_cache;
    std::vector<te_variable> vars;
    
    // Documents
    Nebulite::JSON emptyDoc;          // Linking an empty doc is needed for some functions
    Nebulite::JSON* global = nullptr; // Linkage to global doc

    // pointer to queue
    std::deque<std::string>* tasks = nullptr; 


    //----------------------------------------------------------------
    // Hashmaps and vectors

    // Current and next commands
    // cmds["topic"][]
    absl::flat_hash_map<std::string, std::vector<std::shared_ptr<InvokeEntry>>> globalcommands;
    absl::flat_hash_map<std::string, std::vector<std::shared_ptr<InvokeEntry>>> globalcommandsBuffer; 

    // All pairs of last listens
    absl::flat_hash_map<std::shared_ptr<RenderObject>,std::vector<std::pair<std::shared_ptr<InvokeEntry>,std::shared_ptr<RenderObject>>>> pairs_threadsafe;
    std::vector<std::pair<std::shared_ptr<InvokeEntry>,std::shared_ptr<RenderObject>>> pairs_not_threadsafe;

    // Map for each Tree
    std::shared_mutex exprTreeMutex;
    absl::flat_hash_map<std::string, std::shared_ptr<Invoke::Node>> exprTree;

    
    //----------------------------------------------------------------
    // Private functions

    // For evaluating string expression
    double evaluateExpression(const std::string& expr);

    // Resolving self/other/global references
    std::string resolveVars(const std::string& input, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global);
    

    // Create Tree from string
    std::shared_ptr<Invoke::Node> expressionToTree(const std::string& input);

    // Function for reducing some expressions like 1+1 directly to 2
    void foldConstants(const std::shared_ptr<Invoke::Node>& node);

    // Helper funtion for evaluateNode for parsing 
    std::shared_ptr<Node> parseNext(const std::string& input, size_t& i);

    // Turning a string into a Tree of Nebulite::Invoke::Node
    std::string evaluateNode(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent);

    // Variable access for Node eval
    std::string nodeVariableAccess(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent);
};
}
