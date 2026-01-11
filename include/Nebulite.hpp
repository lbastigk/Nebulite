/**
 * @file Nebulite.hpp
 * @brief Central file for Nebulite namespace documentation and public singleton accessors.
 * @details Offers access to core Nebulite functionality for instances such as DomainModules,
 *          as well as namespace documentation.
 */

#ifndef NEBULITE_HPP
#define NEBULITE_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Core/GlobalSpace.hpp"
#include "Utility/Capture.hpp"
#include "Data/Document/JsonScopeBase.hpp"

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
 * @brief Contains all core classes (called Domains) related to the Nebulite framework.
 * @detauils These form the backbone of the Nebulite system.
 *           They provide essential services such as scoped data management, command parsing, serialization, etc.
 */
namespace Core {
} // namespace Core

/**
 * @namespace Nebulite::Data
 * @brief Contains all fundamental containers used throughout the Nebulite framework.
 */
namespace Data {
} // namespace Data

/**
 * @namespace Nebulite::DomainModule
 * @brief Contains all DomainModules used throughout the Nebulite framework.
 */
namespace DomainModule {
/**
 * @namespace Nebulite::DomainModule::GlobalSpace
 * @brief Contains all GlobalSpace DomainModules.
 */
namespace GlobalSpace {
} // namespace GlobalSpace

/**
 * @namespace Nebulite::DomainModule::JsonScope
 * @brief Contains all JsonScope DomainModules.
 */
namespace JsonScope {
} // namespace JsonScope

/**
 * @namespace Nebulite::DomainModule::Renderer
 * @brief Contains all Renderer DomainModules.
 */
namespace Renderer {
} // namespace Renderer

/**
 * @namespace Nebulite::DomainModule::RenderObject
 * @brief Contains all RenderObject DomainModules.
 */
namespace RenderObject {
} // namespace RenderObject
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
 * @brief Contains all classes, functions, types and variables related to mathematical logic.
 */
namespace Logic {
} // namespace Logic

/**
 * @namespace Nebulite::Interaction::Rules
 * @brief Contains all classes, functions, types and variables related to the
 *        rule-based domain manipulation system in Nebulites Invoke system.
 */
namespace Rules {
/**
 * @namespace Nebulite::Interaction::Rules::Construction
 * @brief Contains all classes, functions, types and variables related to the
 *        construction and initialization of Rulesets/RulesetModules.
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
// Global Document accessor wrapper

namespace Nebulite {

/**
 * @class Nebulite::GlobalDocAccessor
 * @brief Provides access to various scopes within the global document.
 *        Allows access to the document without initializing the entire GlobalSpace.
 */
class GlobalDocAccessor {
public:
    //------------------------------------------
    // Share a read-only setting scope

    [[nodiscard]] Data::JsonScopeBase const& settings() {
        static auto const& settingsScopeConst = globalDoc.shareManagedScopeBase("settings.");
        return settingsScopeConst;
    }

    //------------------------------------------
    // Provide full scope for GlobalSpace Domain

    [[nodiscard]] Core::JsonScope& shareScope(Core::GlobalSpace const& gs, std::string const& prefix) {
        (void)gs; // only used for access control
        return globalDoc.shareManagedScope(prefix);
    }

    //------------------------------------------
    // Provide scopes for DomainModules and RulesetModules, depending on their type

    // GlobalSpace DomainModules root is at "", then we add their own prefix
    [[nodiscard]] Core::JsonScope& shareScope(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm) {
        return globalDoc.shareManagedScope(dm.getDoc().getScopePrefix());
    }

    // Provide a custom scope for DomainModules from RenderObjects
    // We add a prefix to signal what part these domainModules can access
    [[nodiscard]] Core::JsonScope& shareScope(Interaction::Execution::DomainModule<Core::RenderObject> const& dm) {
        return globalDoc.shareManagedScope("providedScope.domainModule.renderObject." + dm.getDoc().getScopePrefix());
    }

    // Provide a custom scope for DomainModules from JsonScope
    // We add a prefix to signal what part these domainModules can access
    [[nodiscard]] Core::JsonScope& shareScope(Interaction::Execution::DomainModule<Core::JsonScope> const& dm) {
        return globalDoc.shareManagedScope("providedScope.domainModule.jsonScope." + dm.getDoc().getScopePrefix());
    }

    // Provide scope to RulesetModules
    [[nodiscard]] Core::JsonScope& shareScope(Interaction::Rules::RulesetModule const& rm) {
        (void)rm; // unused, we provide full scope for now
        // TODO: add a getScopePrefix() to RulesetModule later on if needed
        //       e.g. Physics RulesetModule might only need access to physics-related variables.
        //       For this to work properly, we may have to add the ability to share multiple scopes.
        //       -> physics and time for example
        return globalDoc.shareManagedScope("");
    }
private:
    Data::JSON globalDoc;
};
} // namespace Nebulite

//------------------------------------------
// Singleton accessors

/**
 * @todo: add ErrorTable for access via single header file.
 */
namespace Nebulite {
/**
 * @brief Singleton accessor for the global GlobalSpace object.
 * @details Do not call this in DomainModule constructors, as it may lead to infinite recursion!
 * @return Reference to the global Core::GlobalSpace instance.
 */
Core::GlobalSpace& global();

/**
 * @brief Singleton accessor for the global document.
 * @return Reference to accessor for various scopes within the global document.
 */
GlobalDocAccessor& globalDoc();

/**
 * @brief Singleton accessor for the cout capture object
 * @return CaptureStream object for capturing cout output
 * @todo This is an outdated usage, move to std::print based wrapper later on
 *       -> Nebulite::Log::println(args...);
 *          use recursive variadic templates for that, so that we can have multiple args
 */
Utility::CaptureStream& cout();

/**
 * @brief Singleton accessor for the cerr capture object
 * @return CaptureStream object for capturing cerr output
 * @todo This is an outdated usage, move to std::print based wrapper later on
 *       -> Nebulite::Error::println(args...);
 *          use recursive variadic templates for that, so that we can have multiple args
 */
Utility::CaptureStream& cerr();

/**
 * @brief End line string for capturing output
 *        At the moment, this is just a placeholder for `"\n"`.
 */
inline auto constexpr endl = "\n";

} // namespace Nebulite

//------------------------------------------
// Header file usage detection

#else

// If NEBULITE_HPP is already defined, the file is likely used in a header file context.
// This is discouraged, as Nebulite.hpp is intended to be included only in source files.
// Using it in header files can lead to nasty circular dependencies and lack of encapsulation.

// Toggle between warning and error as needed:
#define NEBULITE_IN_HEADER_BREAK_BUILD

// Issue a warning or error based on the defined macro
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
