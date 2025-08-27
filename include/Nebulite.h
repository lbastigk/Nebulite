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
     * @namespace Nebulite::Constants
     * @brief Contains all constant values used throughout the Nebulite framework.
     */
    namespace Constants{}

    /**
     * @namespace Nebulite::Core
     * @brief Contains all core classes, functions, types and variables related to the Nebulite framework.
     */
    namespace Core{}

    /**
     * @namespace Nebulite::Expansion
     * @brief Contains all classes, functions, types and variables related to the Nebulite expansion system.
     */
    namespace Expansion{
        /**
         * @namespace Nebulite::Expansion::GlobalSpace
         * @brief Contains all classes, functions, types and variables related to the GlobalSpaceTree expansions.
         * See files `GlobalSpaceTree.h` and `GTE_*.h` for usage.
         */
        namespace GlobalSpace{}

        /**
         * @namespace Nebulite::Expansion::RenderObject
         * @brief Contains all classes, functions, types and variables related to the RenderObjectTree expansions.
         * See files `RenderObjectTree.h` and `RTE_*.h` for usage.
         */
        namespace RenderObject{}

        /**
         * @namespace Nebulite::Expansion::JSONTree
         * @brief Contains all classes, functions, types and variables related to the JSONTree expansions.
         * See files `JSONTree.h` and `JTE_*.h` for usage.
         */
        namespace JSON{}

    } // namespace Expansion

    /**
     * @namespace Nebulite::Interaction
     * @brief Contains all classes, functions, types and variables related to the Nebulite interaction system.
     */
    namespace Interaction{  // Invoke itself and its parser

        /**
         * @namespace Nebulite::Interaction::Execution
         * @brief Contains all classes, functions, types and variables related to domain-specific command-processing.
         */
        namespace Execution{}

        /**
         * @namespace Nebulite::Interaction::Logic
         * @brief Contains all classes, functions, types and variables related to the logic processing of Invokes.
         */
        namespace Logic{} // All Logic related classes and functions

    } // namespace Interaction

    /**
     * @namespace Nebulite::Utility
     * @brief Contains all utility classes, functions, types and variables related to the Nebulite framework.
     */
    namespace Utility{}     // JSON, StringHandler, Time, VirtualDouble, Expression


}   // namespace Nebulite