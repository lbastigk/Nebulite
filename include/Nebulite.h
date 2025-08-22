/**
 * @file Nebulite.h
 * 
 * @brief Central file for Nebulite namespace documentation.
 * 
 * While classes/variables have on central definition file, namespaces are only loosely bound.
 * Meaning there isnt a consensus who first defined the namespace.
 * We use a separate file for each namespace to avoid conflicts with doxigen documentation.
 */

 #pragma once

/**
 * @namespace Nebulite
 * 
 * @brief Contains all classes, functions, types and variables related to the Nebulite framework.
 */
namespace Nebulite{

    /**
     * @namespace Nebulite::FuncTreeExpansion
     * @brief Contains all classes, functions, types and variables related to the FuncTree expansions.
     * See files `GTE_*.h`, `RTE_*.h` and `JTE_*.h` for usage.
     */
    namespace FuncTreeExpansion{}

    /**
     * @namespace Nebulite::GlobalSpaceTreeExpansion
     * @brief Contains all classes, functions, types and variables related to the GlobalSpaceTree expansions.
     * See files `GlobalSpaceTree.h` and `GTE_*.h` for usage.
     */
    namespace GlobalSpaceTreeExpansion{}

    /**
     * @namespace Nebulite::RenderObjectTreeExpansion
     * @brief Contains all classes, functions, types and variables related to the RenderObjectTree expansions.
     * See files `RenderObjectTree.h` and `RTE_*.h` for usage.
     */
    namespace RenderObjectTreeExpansion{}

    /**
     * @namespace Nebulite::JSONTreeExpansion
     * @brief Contains all classes, functions, types and variables related to the JSONTree expansions.
     * See files `JSONTree.h` and `JTE_*.h` for usage.
     */
    namespace JSONTreeExpansion{}

}