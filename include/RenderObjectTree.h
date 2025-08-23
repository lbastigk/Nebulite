/**
 * @file RenderObjectTree.h
 * 
 * RenderObjectTree – Function Tree for Local RenderObject Logic
 */


#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"         // Basic Return Type: enum ERROR_TYPE
#include "FuncTree.h"    // All FuncTrees inherit from this for ease of use
#include "FileManagement.h"     // For logging and file operations
#include "JSON.h"

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

/**
 * @class Nebulite::RenderObjectTree
 * @brief This class extends FuncTreeWrapper<ERROR_TYPE> to provide a focused, 
 * self-contained parsing interface (functioncalls) for Nebulite's RenderObject logic.
 * 
 * This allows for Invoke Entries to parse RenderObject-Specific functions, such as:
 * 
 * - align geometry
 * 
 * - logging
 * 
 * - deletion
 * 
 * - Invoke reload
 * 
 * - Updating text
 * 
 * -----------------------------------------------------------
 * 
 * Design Constraints:
 * 
 *     - All functioncalls operate on RenderObjects
 * 
 *     - Access to the global Nebulite JSON
 * 
 *     - For Additional functionality, the usage of Expansion files is encouraged
 *       (see `include/RTE_*.h` for examples)
 * 
 * -----------------------------------------------------------
 * 
 * How to use the RenderObjectTree:
 * 
 *     - Functioncalls are parsed via the Invoke system
 * 
 *     - Create a new Invoke Ruleset through a compatible JSON file
 * 
 *     - add the functioncall to the `functioncalls_self` or `functioncalls_other` array
 * 
 *     - The RenderObjectTree will parse the functioncall and execute it if the invoke is evaluated as true
 * 
 *     - For more advanced features, consider using Expansion files to extend RenderObjectTree functionality
 * 
 */
class RenderObjectTree : public FuncTree<Nebulite::ERROR_TYPE>{
public:
    // Created inside each renderobject, with linkage to the object
    RenderObjectTree(RenderObject* domain, Nebulite::JSONTree* jsonTree);  
    
    void update();
private:
    /**
     * @brief Reference to the domain the FuncTree operates on
     */
    Nebulite::RenderObject* domain;


    /**
     * @brief Factory method for creating expansion instances with proper linkage
     */
    template<typename ExpansionType>
    std::unique_ptr<ExpansionType> createExpansionOfType() {
        auto expansion = std::make_unique<ExpansionType>(domain, this);
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
}   // namespace Nebulite