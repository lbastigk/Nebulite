/**
 * @file ScopeAccessor.hpp
 * @brief Defines the ScopeAccessor class for providing access tokens to various Domain types, Modules, etc.
 */

#ifndef NEBULITE_SCOPE_ACCESSOR_HPP
#define NEBULITE_SCOPE_ACCESSOR_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace; // Forward declaration of GlobalSpace
class JsonScope;   // Forward declaration of JsonScope
class RenderObject; // Forward declaration of RenderObject
} // namespace Nebulite::Core

namespace Nebulite::Interaction::Rules {
class RulesetModule; // Forward declaration of RulesetModule
} // namespace Nebulite::Interaction::Rules

namespace Nebulite::Interaction::Execution {
template<typename DomainType>
class DomainModule; // Forward declaration of DomainModule
} // namespace Nebulite::Interaction::Execution

namespace Nebulite {

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
        std::string prefix = "";
    public:
        std::string getPrefix() const {return prefix; }
    };

    class Full final : public BaseAccessToken {
        Full() {
            prefix = "";
        }

        // Allowed accessors:
        friend class Core::GlobalSpace;
        // TODO: add a helper class in renderer that does the rendering of globalspace using imgui,
        //       then make that class a friend here to allow it access to full scope
    public:
        ~Full() = default;
    };

    // TODO: Build full prefix here for each DomainModule type by using arguments for the constructor
    //       e.g. "providedScope.domainModule.renderObject." + dm.moduleScope.getScopePrefix()
    //       Then we only require one shareScope function that takes the access token and derives the full prefix from it.

    // Provide scoped GlobalSpace access to DomainModules
    class DomainModuleToken {
    public:
        class GlobalSpace final : public BaseAccessToken {
            explicit GlobalSpace(Interaction::Execution::DomainModule<Core::GlobalSpace> const& dm);


            // Allowed accessors:
            friend class Interaction::Execution::DomainModule<Core::GlobalSpace>;
        };

        class RenderObject final : public BaseAccessToken {
            explicit RenderObject(Interaction::Execution::DomainModule<Core::RenderObject> const& dm);


            // Allowed accessors:
            friend class Interaction::Execution::DomainModule<Core::RenderObject>;
        };

        class JsonScope final : public BaseAccessToken {
            explicit JsonScope(Interaction::Execution::DomainModule<Core::JsonScope> const& dm);


            // Allowed accessors:
            friend class Interaction::Execution::DomainModule<Core::JsonScope>;
        };
    };

    // Provide scoped GlobalSpace access to Ruleset-related classes
    class RulesetToken {
    public:
        class RulesetModule final : public BaseAccessToken {
            explicit RulesetModule(Interaction::Rules::RulesetModule const& rm);

            // Allowed accessors:
            friend class Interaction::Rules::RulesetModule;
        };
    };
};

} // namespace Nebulite
#endif // NEBULITE_SCOPE_ACCESSOR_HPP
