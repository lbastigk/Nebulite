/**
 * @file JSONTree.h
 * 
 * JSONTree – Function Tree for Local JSON Logic
 */

#pragma once

//------------------------------------------
// Basic includes
#include "Constants/ErrorTypes.h"  // Basic Return Type: enum ERROR_TYPE
#include "Interaction/Execution/FuncTree.h"    // All FuncTrees inherit from this for ease of use

//------------------------------------------
// Include DomainModules of JSONTree
#include "DomainModule/JSON/JDM_SimpleData.h"
#include "DomainModule/JSON/JDM_ComplexData.h"

//------------------------------------------
// Forward declaration of classes
namespace Nebulite{
    namespace Interaction{
        class Invoke;
    }
    namespace Utility{
        class JSON;
    }
}

namespace Nebulite{
namespace Interaction{
namespace Execution{    
/**
 * @brief This class extends FuncTreeWrapper<Nebulite::Constants::ERROR_TYPE> to provide a focused, 
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
 *     - For Additional functionality, the usage of DomainModule files is encouraged
 *       (see `include/JDM_*.h` for examples)
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
 *     - For more advanced features, consider using DomainModule files to extend JSONTree functionality
 * 
 *  @todo Allow JSONTree to access the global space
 *  This way, we have access to the document cache and can use it to retrieve keys
 */
class JSONTree : public FuncTree<Nebulite::Constants::ERROR_TYPE> {
public:
    JSONTree(Nebulite::Utility::JSON* self);    // Using a raw Pointer itself is fine here, as the Tree is initialized inside JSON

    void update();
private:
    /**
     * @brief Reference to the domain the FuncTree operates on
     */
    Nebulite::Utility::JSON* domain;

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
    // Commands to the JSONTree are added via DomainModule files to keep the JSONTree clean
    // and allow for easy implementation and removal of collaborative features.
    // Maintainers can separately implement their own features and merge them into the JSONTree.
    //
    // 1.) Create a new Class by inheriting from Nebulite::DomainModule::JSON::DomainModuleWrapper ( .h file in ./include and .cpp file in ./src)
    // 2.) Implement the setupBindings() method to bind functions
    // 3.) Insert the new object here as a unique pointer
    // 4.) Initialize via make_unique in the JSONTree constructor
    //------------------------------------------
    std::unique_ptr<Nebulite::DomainModule::JSON::SimpleData> simpleData;
    std::unique_ptr<Nebulite::DomainModule::JSON::ComplexData> complexData;

};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite