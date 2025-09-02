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
     * @namespace Nebulite::DomainModule
     * @brief Contains all classes, functions, types and variables related to the Nebulite DomainModule system.
     */
    namespace DomainModule{
        /**
         * @namespace Nebulite::DomainModule::GlobalSpace
         * @brief Contains all classes, functions, types and variables related to the GlobalSpaceTree DomainModules.
         * See files `GlobalSpaceTree.h` and `GDM_*.h` for usage.
         */
        namespace GlobalSpace{}

        /**
         * @namespace Nebulite::DomainModule::RenderObject
         * @brief Contains all classes, functions, types and variables related to the RenderObjectTree DomainModules.
         * See files `RenderObjectTree.h` and `RDM_*.h` for usage.
         */
        namespace RenderObject{}

        /**
         * @namespace Nebulite::DomainModule::JSONTree
         * @brief Contains all classes, functions, types and variables related to the JSONTree DomainModules.
         * See files `JSONTree.h` and `JDM_*.h` for usage.
         */
        namespace JSON{}

    } // namespace DomainModule

    /**
     * @namespace Nebulite::Interaction
     * @brief Contains all classes, functions, types and variables related to the Nebulite interaction system.
     * 
     * Separated into 2 sub-namespaces: Execution and Logic.
     * 
     * - `Interaction`: main parts of the Interaction system: the Invoke class, their entries and its parser.
     * 
     * - `Interaction::Execution`: All Domain-related parser, the FuncTree they're based on and its DomainModule wrapper
     * 
     * - `Interaction::Logic`: All logic-related classes and functions
     */
    namespace Interaction{

        /**
         * @namespace Nebulite::Interaction::Execution
         * @brief Contains all classes, functions, types and variables related to domain-specific command-processing.
         */
        namespace Execution{}

        /**
         * @namespace Nebulite::Interaction::Logic
         * @brief Contains all classes, functions, types and variables related to mathematical logic in Nebulites Invoke system.
         */
        namespace Logic{}

    } // namespace Interaction

    /**
     * @namespace Nebulite::Utility
     * @brief Contains all utility classes, functions, types and variables related to the Nebulite framework.
     */
    namespace Utility{}


}   // namespace Nebulite