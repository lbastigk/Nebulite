/**
 * @file GlobalSpaceTree.hpp
 * 
 * This file contains the GlobalSpaceTree class, which is the parsing interface for global Nebulite logic.
 */

#pragma once

//------------------------------------------
// Includes

// Nebulite, General
#include "Constants/ErrorTypes.hpp"                             // Basic Return Type: enum ERROR_TYPE
#include "Interaction/Execution/FuncTree.hpp"                   // All FuncTrees inherit from this for ease of use
#include "Interaction/Execution/JSONTree.hpp"                   // For JSONTree parameter

// Nebulite DomainModules of GlobalSpaceTree
#include "DomainModule/GlobalSpace/GDM_General.hpp"             // General functions like eval, exit, wait, etc.
#include "DomainModule/GlobalSpace/GDM_Renderer.hpp"            // Renderer functions for graphics and display
#include "DomainModule/GlobalSpace/GDM_Debug.hpp"               // Debugging and logging functions
#include "DomainModule/GlobalSpace/GDM_GUI.hpp"                 // GUI functions for DearImgui integration
#include "DomainModule/GlobalSpace/GDM_Input.hpp"               // Input handling
#include "DomainModule/GlobalSpace/GDM_RenderObjectDraft.hpp"   // Mock RenderObject for RenderObject functions in global space

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
 *       (see `include/GDM_*.hpp` for examples)
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

    /**
     * @brief Updates internal JSONTree as well as all DomainModules
     */
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
    void createDomainModuleOfType() {
        auto DomainModule = std::make_unique<DomainModuleType>(domain, this);
        modules.push_back(std::move(DomainModule));
    }

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::GlobalSpace>>> modules;
};
}   // namespace Interaction
}   // namespace Execution
}   // namespace Nebulite


