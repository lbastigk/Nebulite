/**
 * @file Debug.hpp
 * @brief Implementation of force and clearForce functions for forcing JSON variable values.
 */

#ifndef MODULE_DOMAIN_COMMON_DEBUG_HPP
#define MODULE_DOMAIN_COMMON_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/DomainModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Interaction::Execution {
class Domain;
} // namespace Nebulite::Interaction::Execution

//------------------------------------------
namespace Nebulite::Module::Domain::Common {
class Debug final : public Base::DomainModule<Interaction::Execution::Domain> {
public:
    [[nodiscard]] Constants::Event updateHook() override;
    void reinit() override {}

    //------------------------------------------
    // Available Functions

    // Fetch

    [[nodiscard]] static Constants::Event fetchId(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr fetchId_name = "fetch-id";
    static auto constexpr fetchId_desc = "Fetches the unique ID of the domain and stores it in the context scope for later use.\n"
        "Usage: fetch-id <key>\n";

    [[nodiscard]] static Constants::Event fetchName(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr fetchName_name = "fetch-name";
    static auto constexpr fetchName_desc = "Fetches the name of the domain and stores it in the context scope for later use.\n"
        "Usage: fetch-name <key>\n";

    [[nodiscard]] static Constants::Event print(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope const& ctxScope);
    static auto constexpr print_name = "print";
    static auto constexpr print_desc = "Prints the JSON document to the console for debugging purposes.\n"
        "If key is empty, prints the entire document.\n"
        "\n"
        "Usage: print [key]\n";

    [[nodiscard]] static Constants::Event printId(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr printId_name = "print-id";
    static auto constexpr printId_desc = "Prints the unique ID of the domain to the console for debugging purposes.\n"
       "Usage: print-id\n";

    // Flow

    [[nodiscard]] Constants::Event warn(std::span<std::string const> const& args) const ;
    static auto constexpr warn_name = "warn";
    static auto constexpr warn_desc = "Sends a warning to the capture.\n"
        "Usage: warn <string>\n"
        "\n"
        "- <string>: The warning message.\n";

    [[nodiscard]] static Constants::Event error(std::span<std::string const> const& args, Interaction::Context const& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr error_name = "error";
    static auto constexpr error_desc = "Echoes all arguments as string to the standard error.\n"
        "Usage: error <string...>\n"
        "\n"
        "- <string...>: One or more strings to echo to the standard error.\n";

    [[noreturn]] static Constants::Event func_throw(std::span<std::string const> const& args);
    static auto constexpr func_throw_name = "throw";
    static auto constexpr func_throw_desc = "Throws a runtime error with the provided message.\n"
        "Usage: throw <string>\n"
        "\n"
        "- <string>: The error message for the thrown exception.\n";

    [[nodiscard]] static Constants::Event mustThrow(std::span<std::string const> const& args, Interaction::Context& ctx, Interaction::ContextScope& ctxScope);
    static auto constexpr mustThrow_name = "must-throw";
    static auto constexpr mustThrow_desc = "Forwards the provided arguments as a function call and expects the function to throw.\n"
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
        bindFunction(&Debug::fetchId, fetchId_name, fetchId_desc);
        bindFunction(&Debug::fetchName, fetchName_name, fetchName_desc);
        bindFunction(&Debug::print, print_name, print_desc);
        bindFunction(&Debug::printId, printId_name, printId_desc);

        // Flow
        bindFunction(&Debug::warn, warn_name, warn_desc);
        bindFunction(&Debug::error, error_name, error_desc);
        bindFunction(&Debug::func_throw, func_throw_name, func_throw_desc);
        bindFunction(&Debug::mustThrow, mustThrow_name, func_throw_desc);
    }
};
} // namespace Nebulite::Module::Domain::Common
#endif // MODULE_DOMAIN_COMMON_DEBUG_HPP
