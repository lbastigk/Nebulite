/**
 * @file General.hpp
 * @brief Class for basic transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_CORE_HPP
#define NEBULITE_TRANSFORMATION_MODULE_CORE_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class General final : public Data::TransformationModule {
public:
    explicit General(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool setString(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr setStringName = "setString";
    static auto constexpr setStringDesc = "Sets a string value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setString <key> <value> -> {json}\n";

    static bool setInt(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr setIntName = "setInt";
    static auto constexpr setIntDesc = "Sets an integer value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setInt <key> <value> -> {json}\n";

    static bool setDouble(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr setDoubleName = "setDouble";
    static auto constexpr setDoubleDesc = "Sets a double value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setDouble <key> <value> -> {json}\n";

    static bool setBool(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr setBoolName = "setBool";
    static auto constexpr setBoolDesc = "Sets a boolean value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value> (true/false).\n"
        "Any other value will be considered false.\n"
        "Usage: |setBool <key> <value> -> {json}\n";

    static bool removeMember(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr removeMemberName = "removeMember";
    static auto constexpr removeMemberDesc = "Removes the member at the specified key from the JSON document.\n"
            "Usage: |removeMember <key1> <key2> ... -> {json}\n";

    static bool setFromResult(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr setFromResultName = "setFromResult";
    static auto constexpr setFromResultDesc = "Sets the value at the specified key in the JSON document from the result of another transformation.\n"
        "Usage: |setFromResult <key> <expression> -> {json}\n"
        "Inside the inner expression, all context is the own scope.\n";

    static bool asString(Data::JsonScope* jsonDoc);
    static auto constexpr asStringName = "asString";
    static auto constexpr asStringDesc = "Converts the current JSON value to a string representation.\n"
        "Usage: |asString -> {string}\n"
        "Either the value as string, or [array] or {object}\n";

    static bool formatNumber(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr formatNumberName = "formatNumber";
    static auto constexpr formatNumberDesc = "If the stored value is a number, it is formatted with a given format specifier\n"
        "Usage: |formatNumber <format> -> {string}"
        "If the value stored is a non-numeric string, the value is not modified.\n"
        "Example formatters: 04.2f, 5i, 06i\n";

    // TODO: setFromExpression?

    // TODO: Even though copy/move is implemented in DomainModule:JsonScope:SimpleData, having these as transformations as well could be useful
    //       instead of writing |parse copy <source> <destination>, we can simply write |copy <source> <destination>
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_CORE_HPP
