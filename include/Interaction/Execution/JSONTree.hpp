/**
 * @file JSONTree.hpp
 * 
 * JSONTree – Function Tree for Local JSON Logic
 */

#pragma once

//------------------------------------------
// Basic includes
#include "Constants/ErrorTypes.hpp"  // Basic Return Type: enum ERROR_TYPE
#include "Interaction/Execution/FuncTree.hpp"    // All FuncTrees inherit from this for ease of use

//------------------------------------------
// Include DomainModules of JSONTree
#include "DomainModule/JSON/JDM_SimpleData.hpp"
#include "DomainModule/JSON/JDM_ComplexData.hpp"

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
 *       (see `include/JDM_*.hpp` for examples)
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
    void createDomainModuleOfType() {
        auto DomainModule = std::make_unique<DomainModuleType>(domain, this);
        modules.push_back(std::move(DomainModule)); // Storing for easier management
    }

    /**
     * @brief Stores all available modules
     */
    std::vector<std::unique_ptr<Nebulite::Interaction::Execution::DomainModule<Nebulite::Utility::JSON>>> modules;

};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite