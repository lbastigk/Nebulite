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

    static bool echo(std::span<std::string const> const& args);
    static auto constexpr echoName = "echo";
    static auto constexpr echoDesc = "Echoes the provided arguments to the console, with newline.\n"
        "Usage: |echo <arg1> <arg2> -> {unchanged-json}\n";

    static bool print(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr printName = "print";
    static auto constexpr printDesc = "Prints the current JSON value to the console.\n"
        "Usage: |print -> {unchanged-json}\n";

    [[noreturn]] static bool unreachable(std::span<std::string const> const& args);
    static auto constexpr unreachableName = "unreachable";
    static auto constexpr unreachableDesc = "Indicates that the current code path should be unreachable.\n"
        "If this transformation is executed, it indicates a bug in the program, and an error message is printed to the console.\n"
        "Usage: |unreachable\n";

    // TODO: log (to file), error (to stderr)
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_DEBUG_HPP
