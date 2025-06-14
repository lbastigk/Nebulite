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

The following is provided to each invoke: 
- self as json doc
  - used to manipulate itself
- other as json doc
  - used to manipulate the other object
  - e.G. self is a solid block with the invoke, other is a moving object
  - other.canMove.Left/Right... can be used to tell the object it cant move in the solid object
- global as json doc
  - count how many wolfes were killed
  - keep track of quest stages: e.g. on pickup, send an invoke to modify quest stage
- a tasklist to write new functioncalls into
  - e.g.: "echo Invoke Activated!"
  - e.g.: "env-deload", "env-load ./Resources/Levels/stage_$($(global.currentStage)+1)"


This also allows to store stuff for other objects to change that are currently not in memory
Like global.levelstate or similiar
*/

// Forward declaration of RenderObject
class RenderObject;

#include <string>
#include <vector>
#include <deque>
#include "tinyexpr.h"
#include "JSON.h"


class Invoke{
public:
    // Static structs
    struct Node {
      // A Type Mix indicates Children!
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
      Type type;
      std::string text;
      std::vector<std::shared_ptr<Invoke::Node>> children; // for nested variables (if Expr)

      // Evaluation optimizations:
      enum class ContextType { None, Self, Other, Global };
      ContextType context = ContextType::None;
      std::string key;
      bool isNumericLiteral = false;
      bool insideEvalParent = false;  // This will be set during evaluation traversal
    };
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

    // Setting up invoke by linking it to a global doc
    Invoke();
    void linkGlobal(Nebulite::JSON& globalDocPtr){
        global = &globalDocPtr;
    }
    void linkQueue(std::deque<std::string>& queue){
        tasks = &queue;
    }

    // Clearing all entries
    void clear();
    
    // Append invoke command
    void append(const std::shared_ptr<InvokeEntry>& toAppend);

    void checkAgainstList(
      const std::shared_ptr<RenderObject>& obj
    );
    bool isTrue(
      const std::shared_ptr<InvokeEntry>& cmd, 
      const std::shared_ptr<RenderObject>& otherObj, 
      bool resolveEqual=true);
    void update();
    void updateGlobal(
      const std::shared_ptr<InvokeEntry>& cmd, 
      const std::shared_ptr<RenderObject>& otherObj
    );
    void updateLocal(
      const std::shared_ptr<InvokeEntry>& cmd
    );
    
    // Get Invokes for next frame
    // Empties current commands, shrinks and swaps with new commands vector.
    void getNewInvokes();

    // Sets new value
    void updateValueOfKey(
      const std::string& type, 
      const std::string& key, 
      const std::string& valStr, 
      Nebulite::JSON *doc
    );

    // For evaluating string expression
    double evaluateExpression(const std::string& expr);
    std::string resolveVars(
      const std::string& input, 
      Nebulite::JSON& self, 
      Nebulite::JSON& other, 
      Nebulite::JSON& global
    );
    std::string resolveGlobalVars(const std::string& input);


    Nebulite::JSON* getGlobalPointer(){return global;};

    std::deque<std::string>* getQueue(){return tasks;};
    
private:
    // TinyExpr
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
    Nebulite::JSON emptyDoc;
    Nebulite::JSON* global = nullptr;

    // Current and next commands
    std::vector<std::shared_ptr<InvokeEntry>> commands;
    std::vector<std::shared_ptr<InvokeEntry>> nextCommands; 
    std::vector<std::pair<std::shared_ptr<InvokeEntry>,std::shared_ptr<RenderObject>>> pairs;

    // pointer to queue
    std::deque<std::string>* tasks = nullptr; 

    // Map for each Tree
    absl::flat_hash_map<std::string, std::shared_ptr<Invoke::Node>> exprTree;

    // Create Tree from string
    std::shared_ptr<Invoke::Node> expressionToTree(const std::string& input);
    void foldConstants(const std::shared_ptr<Invoke::Node>& node);

    std::string evaluateNode(const std::shared_ptr<Invoke::Node>& nodeptr,Nebulite::JSON& self,Nebulite::JSON& other,Nebulite::JSON& global,bool insideEvalParent);
};