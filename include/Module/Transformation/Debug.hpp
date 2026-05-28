/**
 * @file Debug.hpp
 * @brief Class for debug transformation functions.
 */

#ifndef MODULE_TRANSFORMATION_DEBUG_HPP
#define MODULE_TRANSFORMATION_DEBUG_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Debug
 * @brief Various debugging transformations
 * @details These transformations are intended for debugging purposes and may not be suitable for production use.
 *          They can be used to inspect the current JSON value or output messages to the console during transformations.
 */
class Debug final : public Base::TransformationModule {
public:
    explicit Debug(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool echo(std::span<std::string_view const> const& args);
    static auto constexpr echoName = "echo";
    static auto constexpr echoDesc = "Echoes the provided arguments to the console, with newline.\n"
        "Usage: |echo <arg1> <arg2> -> {unchanged-json}\n";

    static bool warn(std::span<std::string_view const> const& args);
    static auto constexpr warnName = "warn";
    static auto constexpr warnDesc = "Echoes the provided arguments to the console as a warning message, with newline.\n"
        "Usage: |warn <arg1> <arg2> -> {unchanged-json}\n";

    static bool error(std::span<std::string_view const> const& args);
    static auto constexpr errorName = "error";
    static auto constexpr errorDesc = "Echoes the provided arguments to the console as an error message, with newline.\n"
        "Usage: |error <arg1> <arg2> -> {unchanged-json}\n";

    static bool print(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr printName = "print";
    static auto constexpr printDesc = "Prints the current JSON value to the console.\n"
        "Usage: |print -> {unchanged-json}\n";

    [[noreturn]] static bool unreachable(std::span<std::string_view const> const& args);
    static auto constexpr unreachableName = "unreachable";
    static auto constexpr unreachableDesc = "Indicates that the current code path should be unreachable.\n"
        "If this transformation is executed, it indicates a bug in the program, and an std::logic_error is thrown.\n"
        "Usage: |unreachable\n";

    static bool store(std::span<std::string_view const> const& args, Data::JsonScope const* jsonDoc);
    static auto constexpr storeName = "store";
    static auto constexpr storeDesc = "Stores the current JSON value as a file with the given name.\n"
        "Usage: |store <filename> -> {unchanged-json}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_DEBUG_HPP
