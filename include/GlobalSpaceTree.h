/*
===========================================================
GlobalSpaceTree - Function Tree for Global Nebulite Logic
===========================================================

This class extends FuncTreeWrapper<ERROR_TYPE> to provide
a focused, self-contained parsing interface (functioncalls)
for Nebulite's global logic.

Unlike RenderObjectTree, which operates on individual
RenderObjects, GlobalSpaceTree handles global operations without affecting
the RenderObject state directly. It is designed for tasks that
require global context, such as:
    - Renderer control
    - General utility functions
    - Debugging and logging
    - Global state management
    - Spawn of RenderObjects

GlobalSpaceTree enables these operations cleanly via keywords
bound to C++ functions, keeping the parsing logic in a separate,
well-scoped layer.

-----------------------------------------------------------
Design Constraints:
    - All functioncalls operate on global Nebulite state
    - No access to individual RenderObject state
    - Restricted to global data and operations
    - For Additional functionality, the usage of Expansion files is encouraged
      (see `include/GTE_*.h` for examples)

-----------------------------------------------------------
How to use the GlobalSpaceTree:
    - Functioncalls are parsed/added to the TaskQueue via the Invoke system
    - Create a new Invoke Ruleset through a compatible JSON file
    - add the functioncall to the "functioncalls_global" array
    - The GlobalSpaceTree will parse the functioncall and execute it if the invoke is evaluated as true
    - For more complex in-object logic, use the RenderObjectTree for local RenderObject operations
    - For more advanced features, consider using Expansion files to extend GlobalSpaceTree functionality
*/

#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTree.h"    // All FuncTrees inherit from this for ease of use
#include "JSONTree.h"           // For JSONTree parameter

//----------------------------------------------------------
// Include Expansions of GlobalSpaceTree
#include "GTE_General.h"    // General functions like eval, exit, wait, etc.
#include "GTE_Renderer.h"   // Renderer functions for graphics and display
#include "GTE_Debug.h"      // Debugging and logging functions

namespace Nebulite{

//----------------------------------------------------------
// Forward declaration of classes
class Invoke;
class GlobalSpace;

//----------------------------------------------------------
// GlobalSpaceTree class, Expand through Expansion files
class GlobalSpaceTree : public FuncTree<Nebulite::ERROR_TYPE>{  // Inherit a funcTree and helper functions
public:
    GlobalSpaceTree(Nebulite::GlobalSpace* self, Nebulite::JSONTree* jsonTree);

    void update();
private:
    // References are needed within the base class to simplify the factory method
    Nebulite::GlobalSpace* self;  // Linkage to the GlobalSpace

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
    // Commands to the GlobalSpaceTree are added via Expansion files to keep the GlobalSpaceTree clean 
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the GlobalSpaceTree. 
    //
    // 1.) Create a new Class by inheriting from Nebulite::GlobalSpaceTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Implement the setupBindings() method to bind functions
    // 3.) Insert the new object here as a unique pointer
    // 4.) Initialize via make_unique in the GlobalSpaceTree constructor
    //---------------------------------------
    std::unique_ptr<GlobalSpaceTreeExpansion::Debug> debug;
    std::unique_ptr<GlobalSpaceTreeExpansion::General> general;
    std::unique_ptr<GlobalSpaceTreeExpansion::Renderer> renderer;
};
}


