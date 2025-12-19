/**
 * @file Nebulite.hpp
 *
 * @brief Central file for Nebulite namespace documentation and public singleton accessors.
 *        Offers access to core Nebulite functionality for instances such as DomainModules
 */

#ifndef NEBULITE_HPP
#define NEBULITE_HPP

//------------------------------------------
// Includes

// Standard library

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Utility/Capture.hpp"

//------------------------------------------
// Namespace documentation

/**
 * @namespace Nebulite
 * @brief Contains all classes, functions, types and variables related to the Nebulite framework.
 */
namespace Nebulite {
/**
 * @namespace Nebulite::Constants
 * @brief Contains all constant values used throughout the Nebulite framework.
 */
namespace Constants {
} // namespace Constants

/**
 * @namespace Nebulite::Core
 * @brief Contains all core classes, functions, types and variables related to the Nebulite framework.
 */
namespace Core {
} // namespace Core

/**
 * @namespace Nebulite::Data
 * @brief Contains all classes, functions, types and variables related to the Nebulite data system.
 */
namespace Data {
} // namespace Data

/**
 * @namespace Nebulite::DomainModule
 * @brief Contains all classes, functions, types and variables related to the Nebulite DomainModule system.
 */
namespace DomainModule {
/**
 * @namespace Nebulite::DomainModule::GlobalSpace
 * @brief Contains all classes, functions, types and variables related to the GlobalSpace DomainModules.
 */
namespace GlobalSpace {
} // namespace GlobalSpace

/**
 * @namespace Nebulite::DomainModule::Renderer
 * @brief Contains all classes, functions, types and variables related to the Renderer DomainModules.
 */
namespace Renderer {
} // namespace Renderer

/**
 * @namespace Nebulite::DomainModule::RenderObject
 * @brief Contains all classes, functions, types and variables related to the RenderObject DomainModules.
 */
namespace RenderObject {
} // namespace RenderObject

/**
 * @namespace Nebulite::DomainModule::JSON
 * @brief Contains all classes, functions, types and variables related to the JSON DomainModules.
 */
namespace JSON {
} // namespace JSON
} // namespace DomainModule

/**
 * @namespace Nebulite::Interaction
 * @brief Contains all classes, functions, types and variables related to the Nebulite interaction system.
 */
namespace Interaction {
/**
 * @namespace Nebulite::Interaction::Execution
 * @brief Contains all classes, functions, types and variables related to domain-specific command-processing.
 */
namespace Execution {
} // namespace Execution

/**
 * @namespace Nebulite::Interaction::Logic
 * @brief Contains all classes, functions, types and variables related to mathematical logic in Nebulites Invoke system.
 */
namespace Logic {
} // namespace Logic

/**
 * @namespace Nebulite::Interaction::Rules
 * @brief Contains all classes, functions, types and variables related to the rule-based manipulation system in Nebulites Invoke system.
 */
namespace Rules {
/**
 * @namespace Nebulite::Interaction::Rules::Construction
 * @brief Contains all classes, functions, types and variables related to the construction and initialization
 *        of Rulesets/RulesetModules.
 */
namespace Construction {
} // namespace Construction

/**
 * @namespace Nebulite::Interaction::Rules::RulesetModules
 * @brief Contains all classes, functions, types and variables related to specific static Rulesets.
 */
namespace RulesetModules {
} // namespace RulesetModules
} // namespace Rules
} // namespace Interaction

/**
 * @namespace Nebulite::Utility
 * @brief Contains all utility classes, functions, types and variables related to the Nebulite framework.
 */
namespace Utility {
} // namespace Utility
} // namespace Nebulite

//------------------------------------------
// Singleton accessors

/**
 * @todo: add ErrorTable for access via single header file.
 */
namespace Nebulite {
/**
 * @brief Singleton accessor for the global GlobalSpace object.
 */
Core::GlobalSpace& global();

/**
 * @brief Singleton accessor for the cout capture object
 * @return CaptureStream object for capturing cout output
 * @todo This is an outdated usage, move to std::print based wrapper later on
 *       -> Nebulite::logln(args...);
 */
Nebulite::Utility::CaptureStream& cout();

/**
 * @brief Singleton accessor for the cerr capture object
 * @return CaptureStream object for capturing cerr output
 * @todo This is an outdated usage, move to std::print based wrapper later on
 *       -> Nebulite::errln(args...);
 */
Nebulite::Utility::CaptureStream& cerr();

/**
 * @brief End line string for capturing output
 *        At the moment, this is just a placeholder for `"\n"`.
 */
inline constexpr const char* endl = "\n";

} // namespace Nebulite
#else

// If NEBULITE_HPP is already defined, the file is likely used in a header file context.
// This is discouraged, as Nebulite.hpp is intended to be included only in source files.
// Using it in header files can lead to nasty circular dependencies and lack of encapsulation.

// Toggle between warning and error as needed:
#define NEBULITE_IN_HEADER_BREAK_BUILD
#ifdef NEBULITE_IN_HEADER_BREAK_BUILD
    // Option 1: Just a warning
    #warning "Likely use of Nebulite.hpp in a header file detected! Including Nebulite.hpp in a header file is discouraged. Please include it only in source files."
#else
    // Option 2: Compile-time error
    // We choose to break the build to enforce this rule.
    // While we won't detect all cases, this will catch many common scenarios.
    #error "Likely use of Nebulite.hpp in a header file detected! Including Nebulite.hpp in a header file is discouraged. Please include it only in source files."
#endif // NEBULITE_IN_HEADER_BREAK_BUILD
#endif // NEBULITE_HPP