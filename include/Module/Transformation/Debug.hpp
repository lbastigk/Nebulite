/**
 * @file Debug.hpp
 * @brief Class for debug transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_DEBUG_HPP
#define NEBULITE_TRANSFORMATION_MODULE_DEBUG_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Module::Transformation {

class Debug final : public Data::TransformationModule {
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

    // TODO: log (to file), error (to stderr)
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_TRANSFORMATION_MODULE_DEBUG_HPP
