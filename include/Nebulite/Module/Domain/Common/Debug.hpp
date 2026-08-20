#ifndef NEBULITE_MODULE_DOMAIN_COMMON_DEBUG_HPP
#define NEBULITE_MODULE_DOMAIN_COMMON_DEBUG_HPP

//------------------------------------------
// Includes

// Standard library
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Constants/Event.hpp"
#include "Nebulite/Interaction/Execution/Domain.hpp"
#include "Nebulite/Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction {
class Context;
class ContextScope;
} // namespace Nebulite::Interaction

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
class Debug final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // Fetch

    [[nodiscard]] static Constants::Event fetchId(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr fetchIdName = "fetch-id";
    static auto constexpr fetchIdDesc = "Fetches the unique ID of the domain and stores it in the context scope for later use.\n"
        "Usage: fetch-id <key>\n";

    [[nodiscard]] static Constants::Event fetchName(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr fetchNameName = "fetch-name";
    static auto constexpr fetchNameDesc = "Fetches the name of the domain and stores it in the context scope for later use.\n"
        "Usage: fetch-name <key>\n";

    [[nodiscard]] static Constants::Event print(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr printName = "print";
    static auto constexpr printDesc = "Prints the JSON document to the console for debugging purposes.\n"
        "If key is empty, prints the entire document.\n"
        "\n"
        "Usage: print [key]\n";

    [[nodiscard]] static Constants::Event printId(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr printIdName = "print-id";
    static auto constexpr printIdDesc = "Prints the unique ID of the domain to the console for debugging purposes.\n"
       "Usage: print-id\n";

    // Flow

    [[nodiscard]] Constants::Event warn(std::span<std::string_view const> args) const ;
    static auto constexpr warnName = "warn";
    static auto constexpr warnDesc = "Sends a warning to the capture.\n"
        "Usage: warn <string>\n"
        "\n"
        "- <string>: The warning message.\n";

    [[nodiscard]] static Constants::Event error(std::span<std::string_view const> args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr errorName = "error";
    static auto constexpr errorDesc = "Echoes all arguments as string to the standard error.\n"
        "Usage: error <string...>\n"
        "\n"
        "- <string...>: One or more strings to echo to the standard error.\n";

    [[noreturn]] static Constants::Event throwFunc(std::span<std::string_view const> args);
    static auto constexpr throwFuncName = "throw";
    static auto constexpr throwFuncDesc = "Throws a runtime error with the provided message.\n"
        "Usage: throw <string>\n"
        "\n"
        "- <string>: The error message for the thrown exception.\n";

    [[nodiscard]] static Constants::Event mustThrow(std::span<std::string_view const> args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr mustThrowName = "must-throw";
    static auto constexpr mustThrowDesc = "Forwards the provided arguments as a function call and expects the function to throw.\n"
        "If the function does not throw, a runtime error is thrown indicating that an exception was expected but not thrown.\n"
        "Usage: must-throw <function call>\n"
        "\n"
        "- <function call>: A function call that is expected to throw an exception.\n";

    //------------------------------------------
    // Setup

    /**
     * @brief Initializes the module, binding functions and variables. 
     */
    explicit Debug(ConstructorParams const& params) : DomainModule(params) {
        // Binding

        // Fetch
        bindFunction(&Debug::fetchId, fetchIdName, fetchIdDesc);
        bindFunction(&Debug::fetchName, fetchNameName, fetchNameDesc);
        bindFunction(&Debug::print, printName, printDesc);
        bindFunction(&Debug::printId, printIdName, printIdDesc);

        // Flow
        bindFunction(&Debug::warn, warnName, warnDesc);
        bindFunction(&Debug::error, errorName, errorDesc);
        bindFunction(&Debug::throwFunc, throwFuncName, throwFuncDesc);
        bindFunction(&Debug::mustThrow, mustThrowName, throwFuncDesc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // NEBULITE_MODULE_DOMAIN_COMMON_DEBUG_HPP
