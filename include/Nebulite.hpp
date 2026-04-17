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
#include "ScopeAccessor.hpp"
#include "Core/GlobalSpace.hpp"
#include "Utility/Capture.hpp"
#include "Data/Document/JsonScope.hpp"

//------------------------------------------
// Global class for singleton access

namespace Nebulite {
/**
 * @brief Static class to provide access to the global GlobalSpace singleton and selected global JSON document scopes.
 */
class Global final {
public:
    /**
     * @brief Provides access to the global GlobalSpace singleton.
     * @return Reference to the global GlobalSpace instance.
     */
    static Core::GlobalSpace& instance();

    /**
     * @brief Provides access to the global settings scope in the global JSON document.
     * @return Constant reference to the settings JsonScope.
     */
    [[nodiscard]] static Data::JsonScope const& settings();

    //------------------------------------------
    // Provide access based on access token and its prefix

    /**
     * @brief Provides access to a shared JsonScope based on the provided access token and prefix.
     * @param at Access token providing the necessary permissions.
     * @param prefix Prefix to append to the access token's prefix for scope retrieval.
     * @return Reference to the shared JsonScope.
     */
    [[nodiscard]] static Data::JsonScope& shareScope(ScopeAccessor::BaseAccessToken const& at, std::string const& prefix = "");

    //------------------------------------------
    // Capture access

    /**
     * @brief Provides access to the global capture instance.
     * @details Whenever possible, use the local capture provided by the Domain (or DomainModule) instead of this global capture, to ensure proper hierarchical capture behavior.
     * @return Reference to the global capture instance.
     */
    static Utility::Capture& capture() {
        return instance().capture;
    }

private:
    /**
     * @brief Provides access to the global JSON document.
     * @return Reference to the global JSON document.
     */
    static Data::JSON& globalDoc();
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
