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
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
// Namespace documentation

/**
 * @namespace Nebulite
 * @brief Contains all classes, functions, types and variables related to the Nebulite framework.
 */
namespace Nebulite {
/**
 * @namespace Nebulite::Constants
 * @brief Contains all constant values/tables used throughout the Nebulite framework.
 */
namespace Constants {
} // namespace Constants

/**
 * @namespace Nebulite::Core
 * @brief Contains all core classes (called Domains) related to the Nebulite framework.
 * @details These form the backbone of the Nebulite system.
 *          They provide essential services such as scoped data management, command parsing, serialization, etc.
 * @note Contains the Texture class, even if it is graphics-related, as it is a core class simply by being a Domain.
 */
namespace Core {
} // namespace Core

/**
 * @namespace Nebulite::Data
 * @brief Contains all fundamental containers used throughout the Nebulite framework.
 */
namespace Data {
/**
 * @namespace Nebulite::Data::TransformationModules
 * @brief Contains all TransformationModules used for modifying JSON values during retrieval.
 */
namespace TransformationModules {
} // namespace TransformationModules
} // namespace Data

/**
 * @namespace Nebulite::DomainModule
 * @brief Contains all DomainModules used throughout the Nebulite framework.
 * @details Further sorted by their associated Domain type.
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
 * @namespace Nebulite::Graphics
 * @brief Contains mainly helper classes to assist with rendering-related tasks.
 */
namespace Graphics {
} // namespace Graphics

/**
 * @namespace Nebulite::Interaction
 * @brief Contains all classes, functions, types and variables related to the Nebulite interaction system.
 */
namespace Interaction {
/**
 * @namespace Nebulite::Interaction::Execution
 * @brief Contains all classes, functions, types and variables related to domain-specific command-processing,
 *        as well as the modulization system through DomainModules.
 */
namespace Execution {
} // namespace Execution

/**
 * @namespace Nebulite::Interaction::Logic
 * @brief Contains all classes, functions, types and variables related to mathematical logic.
 * @details All classes assist in evaluating user-defined runtime logical expressions within the Nebulite framework.
 */
namespace Logic {
} // namespace Logic

/**
 * @namespace Nebulite::Interaction::Rules
 * @brief Contains all classes, functions, types and variables related to the
 *        rule-based domain manipulation system. Both local and global rulesets are handled here.
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
 * @details Compared to json-defined Rulesets, these RulesetModules provide optimized implementations for common use-cases.
 *          They are hardcoded and compiled into the Nebulite framework.
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

namespace Nebulite {

/**
 * @brief Static class to provide access to the global GlobalSpace singleton and selected global JSON document scopes.
 */
class Global {
public:
    //------------------------------------------
    // Provide access to the global GlobalSpace singleton
    static Core::GlobalSpace& instance() {
        return globalSpaceInstance();
    }

    //------------------------------------------
    // Share a read-only setting scope

    [[nodiscard]] static Data::JsonScopeBase const& settings() {
        static auto const& settingsScopeConst = globalDoc().shareManagedScopeBase("settings.");
        return settingsScopeConst;
    }

    //------------------------------------------
    // Define accessor for various scopes

    /**
     * @brief Class to provide access tokens for various Domain types.
     * @details The access tokens can only be constructed by their respective classes.
     * @todo Use this instead of the deprecated shareScope overloads below.
     */
    class ScopeAccessor {
    public:
        class BaseAccessToken {
        protected:
            BaseAccessToken() = default;
            ~BaseAccessToken() = default;
            std::string prefix = "";
        public:
            std::string getPrefix() const {return prefix; }
        };

        class Full final : public BaseAccessToken {
            Full() {
                prefix = "";
            }
            ~Full() = default;

            // Allowed accessors:
            friend class Core::GlobalSpace;
            // TODO: add a helper class in renderer that does the rendering of globalspace using imgui,
            //       then make that class a friend here to allow it access to full scope
        };

        // TODO: Build full prefix here for each DomainModule type by using arguments for the constructor
        //       e.g. "providedScope.domainModule.renderObject." + dm.moduleScope.getScopePrefix()
        //       Then we only require one shareScope function that takes the access token and derives the full prefix from it.

        // Provide GlobalSpace access to DomainModules
        class DomainModuleProvider {
            class GlobalSpace final : public BaseAccessToken {
                explicit GlobalSpace(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm) {
                    prefix = "" + dm.moduleScope.getScopePrefix();
                }
                ~GlobalSpace() = default;

                // Allowed accessors:
                friend class Interaction::Execution::DomainModule<Core::GlobalSpace>;
            };

            class RenderObject final : public BaseAccessToken {
                explicit RenderObject(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm) {
                    prefix = "providedScope.domainModule.renderObject."  + dm.moduleScope.getScopePrefix();
                }
                ~RenderObject() = default;

                // Allowed accessors:
                friend class Interaction::Execution::DomainModule<Core::RenderObject>;
            };

            class JsonScope final : public BaseAccessToken {
                explicit JsonScope(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm) {
                    prefix = "providedScope.domainModule.jsonScope." + dm.moduleScope.getScopePrefix();
                }
                ~JsonScope() = default;

                // Allowed accessors:
                friend class Interaction::Execution::DomainModule<Core::JsonScope>;
            };
        };

        class RulesetModuleProvider {
            class RulesetModule final : public BaseAccessToken {
                explicit RulesetModule(Interaction::Rules::RulesetModule const& rm) {
                    (void)rm; // TODO: add getScopePrefix() to RulesetModule later on
                    prefix = ""; // RulesetModules get full access for now
                }
                ~RulesetModule() = default;

                // Allowed accessors:
                friend class Interaction::Rules::RulesetModule;
            };
        };
    };

    //------------------------------------------
    // Provide access based on access token and its prefix

    [[nodiscard]] static Core::JsonScope& shareScope(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix) {
        return globalDoc().shareManagedScope(at.getPrefix() + prefix);
    }

    //------------------------------------------
    // TODO: The following shareScope overloads are deprecated.

    // Provide scopes for DomainModules and RulesetModules, depending on their type

    // GlobalSpace DomainModules root is at "", then we add their own prefix
    [[nodiscard]] static Core::JsonScope& shareScope(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm) {
        return globalDoc().shareManagedScope(dm.moduleScope.getScopePrefix());
    }

    // Provide a custom scope for DomainModules from RenderObjects
    // We add a prefix to signal what part these domainModules can access
    [[nodiscard]] static Core::JsonScope& shareScope(Interaction::Execution::DomainModule<Core::RenderObject> const& dm) {
        return globalDoc().shareManagedScope("providedScope.domainModule.renderObject." + dm.moduleScope.getScopePrefix());
    }

    // Provide a custom scope for DomainModules from JsonScope
    // We add a prefix to signal what part these domainModules can access
    [[nodiscard]] static Core::JsonScope& shareScope(Interaction::Execution::DomainModule<Core::JsonScope> const& dm) {
        return globalDoc().shareManagedScope("providedScope.domainModule.jsonScope." + dm.moduleScope.getScopePrefix());
    }

    // Provide scope to RulesetModules
    [[nodiscard]] static Core::JsonScope& shareScope(Interaction::Rules::RulesetModule const& rm) {
        (void)rm; // unused, we provide full scope for now
        // TODO: add a getScopePrefix() to RulesetModule later on if needed
        //       e.g. Physics RulesetModule might only need access to physics-related variables.
        //       For this to work properly, we may have to add the ability to share multiple scopes.
        //       -> physics and time for example
        return globalDoc().shareManagedScope("");
    }

    //------------------------------------------
    // Imgui rendering

    static void renderImguiGlobalSettingsWindow();

private:
    // construct-on-first-use singletons to avoid global constructors/destructors
    static Data::JSON& globalDoc() {
        static Data::JSON instance;
        return instance;
    }

    static Core::GlobalSpace& globalSpaceInstance() {
        static Core::GlobalSpace instance{"Nebulite"};
        return instance;
    }
};

class Log {
public:
    template<typename... Args>
    static void print(Args&&... args) {
        Utility::Capture::cout().print(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void println(Args&&... args) {
        Utility::Capture::cout().println(std::forward<Args>(args)...);
    }
};

class Error {
public:
    template<typename... Args>
    static void print(Args&&... args) {
        Utility::Capture::cerr().print(std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void println(Args&&... args) {
        Utility::Capture::cerr().println(std::forward<Args>(args)...);
    }
};

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
