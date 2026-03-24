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
class RenderObject; // Forward declaration of RenderObject
} // namespace Nebulite::Core

namespace Nebulite::Data {
class JsonScope; // Forward declaration of JsonScope
} // namespace Nebulite::Data

namespace Nebulite::DomainModule::Common {
class SimpleData; // Forward declaration of SimpleData DomainModule
} // namespace Nebulite::DomainModule::Common

namespace Nebulite::DomainModule::Renderer {
class Console; // Forward declaration of Console
} // namespace Nebulite::DomainModule::Renderer

namespace Nebulite::Interaction::Rules {
class RulesetModule; // Forward declaration of RulesetModule
} // namespace Nebulite::Interaction::Rules

namespace Nebulite::Interaction::Execution {
template<typename DomainType>
class DomainModule; // Forward declaration of DomainModule
} // namespace Nebulite::Interaction::Execution

namespace Nebulite::Interaction::Logic {
class Expression; // Forward declaration of Expression
} // namespace Nebulite::Interaction::Logic

namespace Nebulite {

//------------------------------------------
// Define accessor for various scopes

/**
 * @brief Class to provide access tokens to the GlobalSpace document for various Domain types.
 * @details The access tokens can only be constructed by their respective classes.
 */
class ScopeAccessor final {
public:
    class BaseAccessToken {
    protected:
        BaseAccessToken() = default;
        std::string prefix;
    public:
        [[nodiscard]] std::string getPrefix() const {return prefix; }
    };

    /**
     * @brief Full access token for the entire GlobalSpace document.
     * @details Only accessible by a select few classes that require full access, such as the Console and Expression evaluation.
     */
    class Full final : public BaseAccessToken {
        Full() {
            prefix = "";
        }

        // Allowed accessors:
        friend class Core::GlobalSpace;
        friend class Interaction::Logic::Expression;
        friend class DomainModule::Renderer::Console; // Console needs full access to display entire scope
        friend class DomainModule::Common::SimpleData; // Required for assign function
    public:
        ~Full() = default;
    };

    // Provide scoped GlobalSpace access to DomainModules
    template<typename DomainType>
    class DomainModuleToken final : public BaseAccessToken {
        explicit DomainModuleToken(Interaction::Execution::DomainModule<DomainType> const& dm);

        // Allowed accessors:
        friend class Interaction::Execution::DomainModule<DomainType>;
    };

    // Provide scoped GlobalSpace access to Ruleset-related classes
    class RulesetModuleToken final : public BaseAccessToken {
        explicit RulesetModuleToken(Interaction::Rules::RulesetModule const& rm);

        // Allowed accessors:
        friend class Interaction::Rules::RulesetModule;
    };
};

} // namespace Nebulite
#include "ScopeAccessor.tpp"
#endif // NEBULITE_SCOPE_ACCESSOR_HPP
