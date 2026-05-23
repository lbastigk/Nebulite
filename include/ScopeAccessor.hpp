/**
 * @file ScopeAccessor.hpp
 * @brief Defines the ScopeAccessor class for providing access tokens to various Domain types, Modules, etc.
 */

#ifndef SCOPEACCESSOR_HPP
#define SCOPEACCESSOR_HPP

//------------------------------------------
// Includes

// Standard library
#include <string>

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Module::Domain::Renderer {
class Console;
} // namespace Nebulite::Module::Domain::Renderer

namespace Nebulite::Module::Base {

class RulesetModule;

template<typename DomainType>
class DomainModule;

} // namespace Nebulite::Module::Base

//------------------------------------------
namespace Nebulite {
/**
 * @brief Class to provide access tokens to the GlobalSpace document for various classes.
 * @details The access tokens can only be constructed by their respective classes.
 *          The access is restricted for some classes, providing only a subscope of the entire document.
 */
class ScopeAccessor final {
public:
    class BaseAccessToken {
    protected:
        BaseAccessToken();
        std::string prefix;
    public:
        [[nodiscard]] std::string const& getPrefix() const ;
    };

    /**
     * @brief Full access token for the entire GlobalSpace document.
     * @details Only accessible by a select few classes that require full access, such as the Console and Expression evaluation.
     */
    class Full final : public BaseAccessToken {
        // Private constructor is intended
        // NOLINTNEXTLINE
        Full();

    public:
        ~Full();

        Full(Full const&) = delete;
        Full& operator=(Full const&) = delete;
        Full(Full&&) = delete;
        Full& operator=(Full&&) = delete;

        // Allowed accessors:
        friend class Core::GlobalSpace; // GlobalSpace needs to create the token and manage access to its Subdomains and itself
        friend class Module::Domain::Renderer::Console; // Console needs full access to display entire scope.
    };

    // Provide scoped GlobalSpace access to DomainModules
    template<typename DomainType>
    class DomainModuleToken final : public BaseAccessToken {
        explicit DomainModuleToken(Module::Base::DomainModule<DomainType> const& dm);

        // Allowed accessors:
        friend class Module::Base::DomainModule<DomainType>;
    };

    // Provide scoped GlobalSpace access to Ruleset-related classes
    class RulesetModuleToken final : public BaseAccessToken {
        explicit RulesetModuleToken(Module::Base::RulesetModule const& rm);

        // Allowed accessors:
        friend class Module::Base::RulesetModule;
    };
};
} // namespace Nebulite
#include "ScopeAccessor.tpp"
#endif // SCOPEACCESSOR_HPP
