/*
===========================================================
MainTree - Function Tree for Global Nebulite Logic
===========================================================

This class extends FuncTreeWrapper<ERROR_TYPE> to provide
a focused, self-contained parsing interface (functioncalls)
for Nebulite's global logic.

Unlike RenderObjectTree, which operates on individual
RenderObjects, MainTree handles global operations without affecting
the RenderObject state directly. It is designed for tasks that
require global context, such as:
    - Renderer control
    - General utility functions
    - Debugging and logging
    - Global state management
    - Spawn of RenderObjects

MainTree enables these operations cleanly via keywords
bound to C++ functions, keeping the parsing logic in a separate,
well-scoped layer.

-----------------------------------------------------------
Design Constraints:
    - All functioncalls operate on global Nebulite state
    - No access to individual RenderObject state
    - Restricted to global data and operations
    - For Additional functionality, the usage of Expansion files is encouraged
      (see `include/MTE_*.h` for examples)

-----------------------------------------------------------
How to use the MainTree:
    - Functioncalls are parsed/added to the TaskQueue via the Invoke system
    - Create a new Invoke Ruleset through a compatible JSON file
    - add the functioncall to the "functioncalls_global" array
    - The MainTree will parse the functioncall and execute it if the invoke is evaluated as true
    - For more complex in-object logic, use the RenderObjectTree for local RenderObject operations
    - For more advanced features, consider using Expansion files to extend MainTree functionality
*/

#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTreeWrapper.h"    // All FuncTrees inherit from this for ease of use

//----------------------------------------------------------
// Include Expansions of MainTree
#include "MTE_General.h"    // General functions like eval, exit, wait, etc.
#include "MTE_Renderer.h"   // Renderer functions for graphics and display
#include "MTE_Debug.h"      // Debugging and logging functions

namespace Nebulite{

//----------------------------------------------------------
// Forward declaration of classes
class Invoke;
class GlobalSpace;

//----------------------------------------------------------
// MainTree class, Expand through Expansion files
class MainTree : public FuncTreeWrapper<Nebulite::ERROR_TYPE>{  // Inherit a funcTree and helper functions
public:
    MainTree(Nebulite::Invoke* invoke, Nebulite::GlobalSpace* globalSpace);
private:
    // References are needed within the base class to simplify the factory method
    Nebulite::Invoke* invokeLinkage;            // Linkage to the Invoke system
    Nebulite::GlobalSpace* globalSpaceLinkage;  // Linkage to the GlobalSpace

    // Factory method for creating expansion instances with proper linkage
    // Improves readability and maintainability
    template<typename ExpansionType>
    std::unique_ptr<ExpansionType> createExpansionOfType() {
        auto expansion = std::make_unique<ExpansionType>(invokeLinkage, globalSpaceLinkage, &funcTree);
        // Initializing is currently done on construction of the expansion
        // However, if any additional setup is needed later on that can't be done on construction,
        // this simplifies the process
        return expansion;
    }

    //---------------------------------------
    // Commands to the MainTree are added via Expansion files to keep the MainTree clean 
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the MainTree. 
    //
    // 1.) Create a new Class by inheriting from Nebulite::MainTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Ensure the Class is a friend of Nebulite::GlobalSpace (see GlobalSpace.h)
    // 3.) Implement the setupBindings() method to bind functions
    // 4.) Insert the new object here as a unique pointer
    // 5.) Initialize via make_unique in the MainTree constructor
    //---------------------------------------
    std::unique_ptr<MainTreeExpansion::Debug> debug;
    std::unique_ptr<MainTreeExpansion::General> general;
    std::unique_ptr<MainTreeExpansion::Renderer> renderer;
};
}


