/**
 * @file Nebulite.hpp
 *
 * @brief Central file for Nebulite namespace documentation. And globally available objects.
 *
 * While classes/variables have on central definition file, namespaces are only loosely bound.
 * Meaning there isnt a consensus who first defined the namespace.
 * We use a separate file for each namespace to avoid conflicts with doxigen documentation.
 * 
 * @todo Since global namespace documentation is not shown in-editor, we may use this central file
 * for globally available objects instead:
 * 
 * - Global ErrorTable object
 * - Global Logger object
 * - Globalspace Object
 */

#ifndef NEBULITE_HPP
#define NEBULITE_HPP

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
         * @brief Contains all classes, functions, types and variables related to the GlobalSpace DomainModules.
         * See files `GlobalSpace.hpp`, `GSDM.hpp` and `GSDM_*.hpp` for usage.
         */
        namespace GlobalSpace{}

        /**
         * @namespace Nebulite::DomainModule::Renderer
         * @brief Contains all classes, functions, types and variables related to the Renderer DomainModules.
         * See files `Renderer.hpp`, `RRDM.hpp` and `RRDM_*.hpp` for usage.
         */
        namespace Renderer{}

        /**
         * @namespace Nebulite::DomainModule::RenderObject
         * @brief Contains all classes, functions, types and variables related to the RenderObject DomainModules.
         * See files `RenderObject.hpp`, `RODM.hpp` and `RODM_*.hpp` for usage.
         */
        namespace RenderObject{}

        /**
         * @namespace Nebulite::DomainModule::JSON
         * @brief Contains all classes, functions, types and variables related to the JSON DomainModules.
         * See files `JSON.hpp`, `JSDM.hpp` and `JSDM_*.hpp` for usage.
         */
        namespace JSON{}

    } // namespace DomainModule

    /**
     * @namespace Nebulite::Interaction
     * @brief Contains all classes, functions, types and variables related to the Nebulite interaction system.
     * 
     * Separated into 2 sub-namespaces: Execution and Logic.
     * 
     * - `Interaction`: main parts of the Interaction system: the Invoke class, their Rulesets and its Compiler.
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
#endif // NEBULITE_HPP