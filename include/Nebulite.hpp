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
#include <cstddef>
#include <deque>
#include <string>

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
}

/**
 * @namespace Nebulite::Core
 * @brief Contains all core classes, functions, types and variables related to the Nebulite framework.
 */
namespace Core {
}

/**
 * @namespace Nebulite::Data
 * @brief Contains all classes, functions, types and variables related to the Nebulite data system.
 */
namespace Data {
}

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
}

/**
 * @namespace Nebulite::DomainModule::Renderer
 * @brief Contains all classes, functions, types and variables related to the Renderer DomainModules.
 */
namespace Renderer {
}

/**
 * @namespace Nebulite::DomainModule::RenderObject
 * @brief Contains all classes, functions, types and variables related to the RenderObject DomainModules.
 */
namespace RenderObject {
}

/**
 * @namespace Nebulite::DomainModule::JSON
 * @brief Contains all classes, functions, types and variables related to the JSON DomainModules.
 */
namespace JSON {
}

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
}

/**
 * @namespace Nebulite::Interaction::Logic
 * @brief Contains all classes, functions, types and variables related to mathematical logic in Nebulites Invoke system.
 */
namespace Logic {
}

/**
 * @namespace Nebulite::Interaction::Rules
 * @brief Contains all classes, functions, types and variables related to the rule-based manipulation system in Nebulites Invoke system.
 */
namespace Rules {
} // namespace Rules

} // namespace Interaction

/**
 * @namespace Nebulite::Utility
 * @brief Contains all utility classes, functions, types and variables related to the Nebulite framework.
 */
namespace Utility {
}

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
 */
Nebulite::Utility::CaptureStream& cout();

/**
 * @brief Singleton accessor for the cerr capture object
 * @return CaptureStream object for capturing cerr output
 */
Nebulite::Utility::CaptureStream& cerr();

/**
 * @brief End line string for capturing output
 *        At the moment, this is just a placeholder for `"\n"`.
 */
static std::string const endl = "\n";

} // namespace Nebulite
#endif // NEBULITE_HPP