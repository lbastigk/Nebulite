/**
 * @file RenderObjectTree.hpp
 * 
 * RenderObjectTree – Function Tree for Local RenderObject Logic
 */


#pragma once

//------------------------------------------
// Basic includes
#include "Constants/ErrorTypes.hpp"               // Basic Return Type: enum ERROR_TYPE
#include "Interaction/Execution/FuncTree.hpp"     // All FuncTrees inherit from this for ease of use
#include "Utility/FileManagement.hpp"             // For logging and file operations
#include "Utility/JSON.hpp"

//------------------------------------------
// Include DomainModules of RenderObjectTree
#include "DomainModule/RenderObject/RDM_Layout.hpp"
#include "DomainModule/RenderObject/RDM_Logging.hpp"
#include "DomainModule/RenderObject/RDM_Parenting.hpp"
#include "DomainModule/RenderObject/RDM_StateUpdate.hpp"

//------------------------------------------
// Forward declaration of classes
namespace Nebulite{
    namespace Core{
        class RenderObject;
    }
}

namespace Nebulite{
namespace Interaction{
namespace Execution{
//------------------------------------------
/**
 * @class Nebulite::Core::RenderObjectTree
 * @brief This class extends FuncTreeWrapper<Nebulite::Constants::ERROR_TYPE> to provide a focused, 
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
 *     - For Additional functionality, the usage of DomainModule files is encouraged
 *       (see `include/RDM_*.hpp` for examples)
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
 *     - For more advanced features, consider using DomainModule files to extend RenderObjectTree functionality
 * 
 */
class RenderObjectTree : public FuncTree<Nebulite::Constants::ERROR_TYPE>{
public:
    // Created inside each renderobject, with linkage to the object
    RenderObjectTree(Nebulite::Core::RenderObject* domain, Nebulite::Interaction::Execution::JSONTree* jsonTree);  
    
    void update();
private:
    /**
     * @brief Reference to the domain the FuncTree operates on
     */
    Nebulite::Core::RenderObject* domain;


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
    std::vector<std::unique_ptr<Nebulite::Interaction::Execution::DomainModule<Nebulite::Core::RenderObject>>> modules;

};
}   // namespace Execution
}   // namespace Interaction
}   // namespace Nebulite