/**
 * @file JSONTree.h
 * 
 * JSONTree – Function Tree for Local JSON Logic
 */

#pragma once

//----------------------------------------------------------
// Basic includes
#include "ErrorTypes.h"  // Basic Return Type: enum ERROR_TYPE
#include "FuncTree.h"    // All FuncTrees inherit from this for ease of use

//----------------------------------------------------------
// Include Expansions of JSONTree
#include "JTE_SimpleData.h"
#include "JTE_ComplexData.h"

namespace Nebulite{
    
// Forward declaration of JSON class
class JSON;

/**
 * @brief This class extends FuncTreeWrapper<ERROR_TYPE> to provide a focused, 
 * self-contained parsing interface (functioncalls) for Nebulite's JSON logic.
 * 
 * This allows for JSON-specific function calls to be parsed and executed within the context of a JSON document,
 * such as:
 *
 * - Copying data
 * 
 * - Modifying keys
 * 
 * - Deleting entries
 * 
 * -----------------------------------------------------------
 * 
 * Design Constraints:
 * 
 *     - All functioncalls operate on JSON documents
 * 
 *     - No Access to global entities, but is planned 
 * 
 *     - For Additional functionality, the usage of Expansion files is encouraged
 *       (see `include/JTE_*.h` for examples)
 * 
 * -----------------------------------------------------------
 * 
 * How to use the JSONTree:
 * 
 *     - Functioncalls are parsed via the Invoke system
 * 
 *     - Create a new Invoke Ruleset through a compatible JSON file
 * 
 *     - add the functioncall to the `functioncalls_self` or `functioncalls_other` array
 * 
 *     - The JSONTree will parse the functioncall just like the RenderObjectTree would and execute it if the invoke is evaluated as true
 * 
 *     - For more advanced features, consider using Expansion files to extend JSONTree functionality
 * 
 *  @todo Allow JSONTree to access the global space
 *  This way, we have access to the document cache and can use it to retrieve keys
 */
class JSONTree : public FuncTree<Nebulite::ERROR_TYPE> {
public:
    JSONTree(JSON* self);    // Using a raw Pointer itself is fine here, as the Tree is initialized inside JSON

    void update();
private:
    /**
     * @brief Reference to the domain the FuncTree operates on
     */
    Nebulite::JSON* domain;

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
    // Commands to the JSONTree are added via Expansion files to keep the JSONTree clean
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the JSONTree.
    //
    // 1.) Create a new Class by inheriting from Nebulite::JSONTreeExpansion::Wrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Implement the setupBindings() method to bind functions
    // 3.) Insert the new object here as a unique pointer
    // 4.) Initialize via make_unique in the JSONTree constructor
    //---------------------------------------
    std::unique_ptr<JSONTreeExpansion::SimpleData> simpleData;
    std::unique_ptr<JSONTreeExpansion::ComplexData> complexData;

};
}   // namespace Nebulite