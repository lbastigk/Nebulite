#pragma once

/*
==== InvokeNode expression parsing structure ====

InvokeNode is an addition to the Invoke system to parse expressions into a compiled tree structure.
This allows for faster evaluation of expressions, as it describes:
- depth of the expression
- type of the expression part (literal, variable, mix_eval, mix_no_eval)
- context of the expression part (variable from self, other, global, resources)
- cast type of the expression part (float, int, none)


Each expression is pre-processed using a Tree build of Nodes
Each node is a part of the expression:
- Literal:     "this is a literal"
- Variable:    "$(global.time.t)"
- Mix_eval:    "$(1+$(global.time.t))"
- Mix_no_eval: "The time is: $(global.time.t)"

Type Mix indicates Children!

Example, say self.variable = 2

Version 1: Mix_eval
$($(self.variable) + 1)
Root has a $(...)
- children_1 : text = self.variable, type = Variable
- children_2 : text = " + 1",        type = Literal
Result: "3"


Version 1: Mix_no_eval
$(self.variable) + 1
Root has no $(...)
- children[0] : text = self.variable, type = Variable
- children[1] : text = " + 1",        type = Literal
Result: "2 + 1"
*/

#include <string>
#include <vector>
#include <deque>
#include "JSON.h"

namespace Nebulite{

// Forward declaration
class Invoke;

// Compiled expression, a collection of Nodes forming a tree structure
class InvokeNode{
public:
    enum class Type {
        Literal,      // Plain text
        Variable,     // $(self.value) or similar
        Mix_eval,     // An expression like $($(self.var) + 1), must evaluate entire subtree
        Mix_no_eval   // A mix of variables and literals, but not wrapped in $(...), just concatenate
    };
    enum class CastType { None, Float, Int };
    enum class ContextType { None, Self, Other, Global, Resources };

    // Since this is now a class instead of a struct, we need to define the constructor
    InvokeNode() = default;
    InvokeNode(Type type, const std::string& text = "", const std::vector<std::shared_ptr<InvokeNode>>& children = {})
        : type(type), text(text), children(children) {}
    InvokeNode(Type type, const std::string& text, const std::vector<std::shared_ptr<InvokeNode>>& children, ContextType context, CastType cast = CastType::None)
        : type(type), text(text), children(children), context(context), cast(cast) {}
    
    friend class InvokeNodeHelper; // Allow InvokeNodeHelper to access private members
private:
    // Data type
    Type type = Type::Literal;

    // Text content of the node, e.g.:
    // "Hello World" for literals
    // self.variable for variables
    // <empty>       for mix_eval and mix_no_eval
    std::string text;

    // for nested nodes, e.g. $($(self.variable) + 1)
    std::vector<std::shared_ptr<Nebulite::InvokeNode>> children; 
    
    // Context on what data resource is taken from
    ContextType context = ContextType::None;    

    // Cast type. E.g.: $f(...) or $i(...)
    CastType cast = CastType::None; 

    // doc[key] being used TODO: combineable with text?
    //std::string key;    

    // If the node contains just a number: $(100)
    // This reduces overhead, as no evaluation is needed
    bool isNumericLiteral = false;  

    // Making evaluation faster by checking if a parent would evaluate this node:
    // e.g.: $(1 + $(2 + 3))
    // makes sure that the inner $(2+3) expression is not resolved:
    // $(1 + 5.0000)  -> BAD, total calls for expr is 2
    // $(1 + (2 + 3)) -> BETTER. only one call
    bool insideEvalParent = false;      
};   

// Used to maniupulate InvokeNode objects, must be linked to Invoke instance
// to access global variables and methods
class InvokeNodeHelper{
public:
    InvokeNodeHelper(Nebulite::Invoke* invoke) : invoke(invoke) {}

    // turn nodes that hold just constant to evaluate into text
    // e.g. $(1+1) is turned into 2.000...
    //
    // TODO: More testing needed if this function is helpful/works correctly
    // While evaluation works, it is not clear if this function actually does fold constants...
    // Testing example:
    // - Add debug statements on reduction
    // - call: ./bin/Nebulite eval echo '$(1+1)'
    //   Should fold Node from Mix_eval to Literal, then print 2
    void foldConstants(const std::shared_ptr<Nebulite::InvokeNode>& node);

    // Main function for turning a string expression into a Node Tree
    std::shared_ptr<Nebulite::InvokeNode> expressionToTree(const std::string& input);

    // Helper function for casting a node to a string value with a given cast type
    // e.g. for node with text "3.14" and cast type Float,
    // this will return "3.140000" as a string
    // If the cast type is int, it will return "3"
    std::string castValue(const std::string& value, Nebulite::InvokeNode* nodeptr, Nebulite::JSON *doc);

    // Helper function to parse inner variable and setting the context
    // - Self/Other/Global/Resources access
    // - Numeric literal
    Nebulite::InvokeNode parseInnerVariable(const std::string& inner);

    // Helper funtion for evaluateNode for parsing 
    std::shared_ptr<Nebulite::InvokeNode> parseChild(const std::string& input, size_t& i);

    // Takes a pre-processed node and combines all children into a single string
    std::string combineChildren(const std::shared_ptr<Nebulite::InvokeNode>& nodeptr, Nebulite::JSON *self, Nebulite::JSON *other, Nebulite::JSON *global, bool insideEvalParent);

    // Take a pre-processed node and resolve all expressions and vars of this and nodes below
    //
    // Examples:
    // $($(global.constants.pi) + 1)  -> 4.141..
    //   $(global.constants.pi) + 1   -> 3.141... + 1
    // Time is: $(global.time.t)      -> Time is: 11.01
    std::string evaluateNode(const std::shared_ptr<Nebulite::InvokeNode>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent);

    // Helper function for accessing a variable from self/other/global/Resources
    std::string nodeVariableAccess(const std::shared_ptr<Nebulite::InvokeNode>& nodeptr,Nebulite::JSON *self,Nebulite::JSON *other,Nebulite::JSON *global,bool insideEvalParent);
private:
    Nebulite::Invoke* invoke; // Pointer to the Invoke instance for accessing global variables and methods
};
}