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
    explicit General(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool setString(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr setStringName = "setString";
    static auto constexpr setStringDesc = "Sets a string value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setString <key> <value> -> {json}\n";

    static bool setInt(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr setIntName = "setInt";
    static auto constexpr setIntDesc = "Sets an integer value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setInt <key> <value> -> {json}\n";

    static bool setDouble(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr setDoubleName = "setDouble";
    static auto constexpr setDoubleDesc = "Sets a double value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setDouble <key> <value> -> {json}\n";

    static bool setBool(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr setBoolName = "setBool";
    static auto constexpr setBoolDesc = "Sets a boolean value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value> (true/false).\n"
        "Any other value will be considered false.\n"
        "Usage: |setBool <key> <value> -> {json}\n";

    static bool removeMember(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr removeMemberName = "removeMember";
    static auto constexpr removeMemberDesc = "Removes the member at the specified key from the JSON document.\n"
            "Usage: |removeMember <key1> <key2> ... -> {json}\n";

    bool setFromResult(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) const ;
    static auto constexpr setFromResultName = "setFromResult";
    static auto constexpr setFromResultDesc = "Sets the value at the specified key in the JSON document from the result of another transformation.\n"
        "Usage: |setFromResult <key> {!transformation} -> {json}\n"
        "The '!' is required, otherwise the nested variable is evaluated by the expression class before the transformation is applied!\n";

    // TODO: setFromExpression?
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_CORE_HPP
