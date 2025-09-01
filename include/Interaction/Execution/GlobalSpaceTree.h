/**
 * @file GlobalSpaceTree.h
 * 
 * This file contains the GlobalSpaceTree class, which is the parsing interface for global Nebulite logic.
 */

#pragma once

//------------------------------------------
// Basic includes
#include "Constants/ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "Interaction/Execution/FuncTree.h"    // All FuncTrees inherit from this for ease of use
#include "Interaction/Execution/JSONTree.h"           // For JSONTree parameter

//------------------------------------------
// Include DomainModules of GlobalSpaceTree
#include "DomainModule/GlobalSpace/GDM_General.h"    // General functions like eval, exit, wait, etc.
#include "DomainModule/GlobalSpace/GDM_Renderer.h"   // Renderer functions for graphics and display
#include "DomainModule/GlobalSpace/GDM_Debug.h"      // Debugging and logging functions
#include "DomainModule/GlobalSpace/GDM_GUI.h"        // GUI functions for DearImgui integration
#include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.h" // Mock RenderObject for RenderObject functions in global space

//------------------------------------------
// Forward declaration of classes
namespace Nebulite{
    namespace Interaction{
        class Invoke;
    }
    namespace Core{
        class GlobalSpace;
    }
}

namespace Nebulite{
namespace Interaction{
namespace Execution{
//------------------------------------------
/**
 * @class Nebulite::Interaction::Execution::GlobalSpaceTree
 * @brief This class extends FuncTreeWrapper<Nebulite::Constants::ERROR_TYPE> to provide a focused, 
 * self-contained parsing interface (functioncalls) for Nebulite's global logic.
 * 
 * Unlike RenderObjectTree, which operates on individual
 * RenderObjects, GlobalSpaceTree handles global operations without affecting
 * the RenderObject state directly. It is designed for tasks that
 * require global context, such as:
 * 
 *     - Renderer control
 * 
 *     - General utility functions
 * 
 *     - Debugging and logging
 * 
 *     - Global state management
 * 
 *     - Spawn of RenderObjects
 * 
 * GlobalSpaceTree enables these operations cleanly via keywords
 * bound to C++ functions, keeping the parsing logic in a separate,
 * well-scoped layer.
 * 
 * -----------------------------------------------------------
 * 
 * Design Constraints:
 *     - All functioncalls operate on global Nebulite state
 *     - No access to individual RenderObject state
 *     - Restricted to global data and operations
 *     - For Additional functionality, the usage of DomainModule files is encouraged
 *       (see `include/GDM_*.h` for examples)
 * 
 * -----------------------------------------------------------
 * 
 * How to use the GlobalSpaceTree:
 * 
 *     - Functioncalls are parsed/added to the TaskQueue via the Invoke system
 * 
 *     - Create a new Invoke Ruleset through a compatible JSON file
 * 
 *     - add the functioncall to the `functioncalls_global` array
 * 
 *     - The GlobalSpaceTree will parse the functioncall and execute it if the invoke is evaluated as true
 * 
 *     - For more complex in-object logic, use the RenderObjectTree for local RenderObject operations
 * 
 *     - For more advanced features, consider using DomainModule files to extend GlobalSpaceTree functionality
 * 
 */
class GlobalSpaceTree : public Nebulite::Interaction::Execution::FuncTree<Nebulite::Constants::ERROR_TYPE>{
public:
    /**
     * @todo: Possible to have constructor in header?
     * Would simplify implementation
     */
    GlobalSpaceTree(Nebulite::Core::GlobalSpace* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree);

    void update();
private:
    /**
     * @brief Reference to the domain the FuncTree operates on
     */
    Nebulite::Core::GlobalSpace* domain;

    /**
     * @brief Factory method for creating DomainModule instances with proper linkage
     */
    template<typename DomainModuleType>
    std::unique_ptr<DomainModuleType> createDomainModuleOfType() {
        auto DomainModule = std::make_unique<DomainModuleType>(domain, this);
        // Initializing is currently done on construction of the DomainModule
        // However, if any additional setup is needed later on that can't be done on construction,
        // this simplifies the process
        return DomainModule;
    }

    //------------------------------------------
    // Commands to the GlobalSpaceTree are added via DomainModule files to keep the GlobalSpaceTree clean 
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the GlobalSpaceTree. 
    //
    // 1.) Create a new Class by inheriting from Nebulite::DomainModule::GlobalSpace::DomainModuleWrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Implement the setupBindings() method to bind functions
    // 3.) Insert the new object here as a unique pointer
    // 4.) Initialize via make_unique in the GlobalSpaceTree constructor
    //------------------------------------------
    std::unique_ptr<Nebulite::DomainModule::GlobalSpace::Debug> debug;                             // Debugging functions such as logging, creating standard files etc.
    std::unique_ptr<Nebulite::DomainModule::GlobalSpace::General> general;                         // General functions such as echo, exit, task loading etc.
    std::unique_ptr<Nebulite::DomainModule::GlobalSpace::Renderer> renderer;                       // Renderer DomainModule for global rendering control
    std::unique_ptr<Nebulite::DomainModule::GlobalSpace::GUI> gui;                                 // GUI DomainModule for DearImgui integration
    std::unique_ptr<Nebulite::DomainModule::GlobalSpace::RenderObjectDraft> RenderObjectDraft;     // Mock RenderObject for testing purposes
};
}   // namespace Interaction
}   // namespace Execution
}   // namespace Nebulite


