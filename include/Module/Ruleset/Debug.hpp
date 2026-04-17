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
    static std::string_view constexpr messageName = "::debug::message";
    static std::string_view constexpr messageDesc = "Outputs a debug message to the standard output (cout).";

    // Provides a simple debug message to cerr
    void error(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr errorName = "::debug::error";
    static std::string_view constexpr errorDesc = "Outputs a debug error message to the standard error output (cerr).";

    // Prints the ids of both contexts
    void whoInteracts(Interaction::Context const& context, double**& slf, double**& otr) const ;
    static std::string_view constexpr whoInteractsName = "::debug::whoInteracts";
    static std::string_view constexpr whoInteractsDesc = "Prints a message with the unique IDs of the self and other render object contexts.";

    //------------------------------------------
    // Constructor
    Debug() : RulesetModule(moduleName) {
        auto const baseListFunc = generateBaseListFunction(baseKeys);

        // Local
        bind<messageName>(RulesetType::Local, &Debug::message, messageDesc, baseListFunc);
        bind<errorName>(RulesetType::Local, &Debug::error, errorDesc, baseListFunc);

        // Global
        bind<whoInteractsName>(RulesetType::Global, &Debug::whoInteracts, whoInteractsDesc, baseListFunc);
    }

    static std::string_view constexpr moduleName = "::debug";
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
