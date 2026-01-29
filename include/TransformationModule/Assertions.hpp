/**
 * @file Assertions.hpp
 * @brief Class for assertion transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP
#define NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Assertions final : public Data::TransformationModule {
public:
    explicit Assertions(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool assertNonEmpty(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr assertNonEmptyName = "assertNonEmpty";
    static auto constexpr assertNonEmptyDesc = "Asserts that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails and the program exits\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |assertNonEmpty -> {value,<Exception thrown if empty>}\n";

    // TODO: assertType, assertRange, assertRegex, etc.

    // TODO: Another class for silent assertions that do not exit the program, but return false on failure
    //       e.g., requiresNonEmpty, requiresType, requiresRange, requiresRegex, etc.

private:
    /**
     * @brief Prints the message inside its arguments.
     * @details Assumes the first arg is the caller function name.
     * @param args Arguments containing the message parts.
     */
    static void printUserDefinedMessage(std::span<std::string const> const& args);
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_ASSERTIONS_HPP
