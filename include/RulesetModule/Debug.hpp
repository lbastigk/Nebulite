/**
*  @file Debug.hpp
 * @brief Debugging ruleset module for Nebulite interaction system.
 */

#ifndef NEBULITE_RULESET_MODULE_DEBUG_HPP
#define NEBULITE_RULESET_MODULE_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library

// External

// Nebulite
#include "Interaction/Rules/RulesetModule.hpp"

//------------------------------------------
namespace Nebulite::RulesetModule {
class Debug : public Interaction::Rules::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Provides a simple debug message to cout
    void message(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static constexpr std::string_view messageName = "::debug::message";
    static constexpr std::string_view messageDesc = "Outputs a debug message to the standard output (cout).";

    // Provides a simple debug message to cerr
    void error(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static constexpr std::string_view errorName = "::debug::error";
    static constexpr std::string_view errorDesc = "Outputs a debug error message to the standard error output (cerr).";

    // Prints the ids of both contexts
    void whoInteracts(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static constexpr std::string_view whoInteractsName = "::debug::whoInteracts";
    static constexpr std::string_view whoInteractsDesc = "Prints a message with the unique IDs of the self and other render object contexts.";

    //------------------------------------------
    // Constructor
    Debug() : RulesetModule(moduleName) {
        std::function<double**(const Interaction::Execution::Domain&)> const baseListFunc = [this](const Interaction::Execution::Domain& domain) -> double** {
            double** v;
            ensureBaseList(domain, baseKeys, v);
            return v;
        };

        // Local
        bind<messageName>(RulesetType::Local, &Debug::message, messageDesc, baseListFunc);
        bind<errorName>(RulesetType::Local, &Debug::error, errorDesc, baseListFunc);

        // Global
        bind<whoInteractsName>(RulesetType::Global, &Debug::whoInteracts, whoInteractsDesc, baseListFunc);
    }

    static constexpr std::string_view moduleName = "::debug";
private:
    //------------------------------------------
    // Base value caching

    const std::vector<Data::ScopedKeyView> baseKeys = {
        // No keys required
    };

    enum class Key : size_t {
        // No keys required
    };
};
} // namespace Nebulite::RulesetModule
#endif // NEBULITE_RULESET_MODULE_DEBUG_HPP
