#ifndef NEBULITE_SCOPEACCESSOR_HPP
#define NEBULITE_SCOPEACCESSOR_HPP

//------------------------------------------
// Includes

// Standard library
#include <optional>
#include <string>

//------------------------------------------
// Forward declarations

namespace Nebulite::Core {
class GlobalSpace;
class RenderObject;
} // namespace Nebulite::Core

namespace Nebulite::Interaction {
class Invoke;
} // namespace Nebulite::Interaction

namespace Nebulite::Module::Domain::Renderer {
class Console;
} // namespace Nebulite::Module::Domain::Renderer

namespace Nebulite::Module::Base {
class RulesetModule;

template<typename DomainType>
class DomainModule;

} // namespace Nebulite::Module::Base

//------------------------------------------
/**
 * @namespace Nebulite::ScopeAccessor
 * @brief Provide access tokens to the GlobalSpace document for various classes.
 * @details The access tokens can only be constructed by their respective classes.
 *          The access is restricted for some classes, providing only a subscope of the entire document.
 */
namespace Nebulite::ScopeAccessor {
class BaseAccessToken {
protected:
    BaseAccessToken();
    std::optional<std::string> prefix;
public:
    [[nodiscard]] std::optional<std::string> const& getPrefix() const ;
};

/**
 * @brief Full access token for the entire GlobalSpace document.
 * @details Only accessible by a select few classes that require full access, such as the Console and Expression evaluation.
 */
class Full final : public BaseAccessToken {
    // Private constructor is intended
    Full(); // NOLINT
    ~Full(); // NOLINT

public:
    Full(Full const&) = delete;
    Full& operator=(Full const&) = delete;
    Full(Full&&) = delete;
    Full& operator=(Full&&) = delete;

    // Allowed accessors:
    friend class Core::GlobalSpace; // GlobalSpace needs to create the token and manage access to its Subdomains and itself
    friend class Module::Domain::Renderer::Console; // Console needs full access to display entire scope.
    friend class Interaction::Invoke;
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
} // namespace Nebulite::ScopeAccessor
#include "Nebulite/ScopeAccessor.tpp" // NOLINT(misc-include-cleaner)
#endif // NEBULITE_SCOPEACCESSOR_HPP
