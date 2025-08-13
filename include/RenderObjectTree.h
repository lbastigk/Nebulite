/*
===========================================================
RenderObjectTree – Function Tree for Local RenderObject Logic
===========================================================

This class extends FuncTreeWrapper<ERROR_TYPE> to provide
a focused, self-contained parsing interface (functioncalls)
for `RenderObject`.

Unlike global invoke entries (which allow inter-object logic),
functioncalls in RenderObjectTree operate *exclusively* on
the RenderObject they are attached to (the "self" object).

-----------------------------------------------------------
Why is this layer needed in addition to GlobalSpaceTree?

While the GlobalSpaceTree handles global operations, the RenderObjectTree
is designed for tasks that require local context, such as:
    - flagging an object for deletion
    - changing layout properties
    - internal debugging through logging
    - complex data management within the RenderObject

-----------------------------------------------------------
Why is this layer needed in addition to Invokes?

Invokes are general-purpose and powerful, but they are best
used for dataflow and inter-object logic. However, some tasks:
    - require more complex conditional logic,
    - are hard to express as value changes alone,
    - need tight control over internal render object state.

RenderObjectTree enables these operations cleanly via keywords
bound to C++ functions, keeping the parsing logic in a separate,
well-scoped layer.

This system allows you to:
    - Simplify invoke rules
    - Remove clutter like `flag_delete` within the RenderObject JSON doc
    - Add utility logic for layout and state changes
    - Log/debug self object behavior locally, allowing for easier debugging without recompiling binaries

-----------------------------------------------------------
Design Constraints:
    - All functioncalls operate on `self` (the attached RenderObject)
    - No access to global values (use the usual Invoke expression system for self-other-global logic)
    - Restricted to linked data: `self` and its direct children
    - For Additional functionality, the usage of Expansion files is encouraged
      (see `include/RTE_*.h` for examples)

-----------------------------------------------------------
How to use the RenderObjectTree:
    - Functioncalls are parsed/added to the TaskQueue via the Invoke system
    - Create a new Invoke Ruleset through a compatible JSON file
    - add the functioncall to the "functioncalls_self" or "functioncalls_other" array
    - The RenderObjectTree will parse the functioncall and execute it if the invoke is evaluated as true
    - For more complex global logic, use the GlobalSpaceTree for global operations
    - For more advanced features, consider using Expansion files to extend RenderObjectTree functionality
*/


#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTree.h"    // All FuncTrees inherit from this for ease of use
#include "FileManagement.h"     // For logging and file operations

//----------------------------------------------------------
// Include Expansions of RenderObjectTree
#include "RTE_Layout.h"
#include "RTE_Logging.h"
#include "RTE_Parenting.h"
#include "RTE_StateUpdate.h"

namespace Nebulite{

//----------------------------------------------------------
// Forward declaration of classes
class RenderObject;

//----------------------------------------------------------
// RenderObjectTree class, Expand through Expansion files
class RenderObjectTree : public FuncTree<Nebulite::ERROR_TYPE>{  // Inherit a funcTree and helper functions
public:
    // Created inside each renderobject, with linkage to the object
    RenderObjectTree(RenderObject* self, Nebulite::JSONTree* jsonTree);  
    
    void update();
private:
    // Self-reference to the RenderObject is needed within the base class to simplify the factory method
    RenderObject* self;  // Store reference to self

    // Factory method for creating expansion instances with proper linkage
    // Improves readability and maintainability
    template<typename ExpansionType>
    std::unique_ptr<ExpansionType> createExpansionOfType() {
        auto expansion = std::make_unique<ExpansionType>(self, this);
        // Initializing is currently done on construction of the expansion
        // However, if any additional setup is needed later on that can't be done on construction,
        // this simplifies the process
        return expansion;
    }


    //---------------------------------------
    // Commands to the RenderObjectTree are added via Expansion files to keep the RenderObjectTree clean
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the RenderObjectTree.
    //
    // 1.) Create a new Class by inheriting from Nebulite::RenderObjectTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Implement the setupBindings() method to bind functions
    // 3.) Insert the new object here as a unique pointer
    // 4.) Initialize via make_unique in the RenderObjectTree constructor
    //---------------------------------------
    std::unique_ptr<RenderObjectTreeExpansion::Layout> layout;
    std::unique_ptr<RenderObjectTreeExpansion::Logging> logging;
    std::unique_ptr<RenderObjectTreeExpansion::Parenting> parenting;
    std::unique_ptr<RenderObjectTreeExpansion::StateUpdate> stateUpdate;

};
}