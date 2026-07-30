#ifndef NEBULITE_MODULE_RULESET_DEBUG_HPP
#define NEBULITE_MODULE_RULESET_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library
#include <array>
#include <cstdint>
#include <string_view>

// Nebulite
#include "Nebulite/Data/Document/ScopedKeyView.hpp"
#include "Nebulite/Interaction/Rules/Ruleset.hpp"
#include "Nebulite/Module/Base/RulesetModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Ruleset {
class Debug : public Base::RulesetModule {
public:
    //------------------------------------------
    // Functions

    // Provides a simple debug message to cout
    void message(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr messageName = "::debug::message";
    static std::string_view constexpr messageDesc = "Outputs a debug message to the standard output (cout).";

    // Provides a simple debug message to cerr
    void error(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr errorName = "::debug::error";
    static std::string_view constexpr errorDesc = "Outputs a debug error message to the standard error output (cerr).";

    // Prints the ids of both contexts
    void whoInteracts(Interaction::Context const& context, double** slf, double** otr) const ;
    static std::string_view constexpr whoInteractsName = "::debug::whoInteracts";
    static std::string_view constexpr whoInteractsDesc = "Prints a message with the unique IDs of the self and other render object contexts.";

    //------------------------------------------
    // Constructor
    Debug() : RulesetModule(moduleName, this) {
        auto const baseListFunc = generateBaseListFunction(baseKeys);

        // Local
        bind<messageName>(&Debug::message, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, messageDesc);
        bind<errorName>(&Debug::error, baseListFunc, Interaction::Rules::StaticRuleset::Type::local, errorDesc);

        // Global
        bind<whoInteractsName>(&Debug::whoInteracts, baseListFunc, Interaction::Rules::StaticRuleset::Type::global, whoInteractsDesc);
    }

    static std::string_view constexpr moduleName = "::debug";

    //------------------------------------------
    // Base value caching

    static std::array<Data::ScopedKeyView, 0> constexpr baseKeys = {}; // No keys required

    enum class Key : std::uint8_t {}; // No keys required
};
} // namespace Nebulite::Module::Ruleset
#endif // NEBULITE_MODULE_RULESET_DEBUG_HPP
