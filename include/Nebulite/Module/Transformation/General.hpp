#ifndef NEBULITE_MODULE_TRANSFORMATION_GENERAL_HPP
#define NEBULITE_MODULE_TRANSFORMATION_GENERAL_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Interaction/Execution/FuncTree.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::General
 * @brief Basic transformation functions: setting, assigning, removing, formatting, etc.
 */
class General final : public Base::TransformationModule {
public:
    explicit General(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool setString(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr setStringName = "setString";
    static auto constexpr setStringDesc = "Sets a string value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setString <key> <value> -> {json}\n";

    static bool setInt(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr setIntName = "setInt";
    static auto constexpr setIntDesc = "Sets an integer value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setInt <key> <value> -> {json}\n";

    static bool setDouble(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr setDoubleName = "setDouble";
    static auto constexpr setDoubleDesc = "Sets a double value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value>.\n"
        "Usage: |setDouble <key> <value> -> {json}\n";

    static bool setBool(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr setBoolName = "setBool";
    static auto constexpr setBoolDesc = "Sets a boolean value at the specified key in the JSON document.\n"
        "Expects two arguments: <key> and <value> (true/false).\n"
        "Any other value will be considered false.\n"
        "Usage: |setBool <key> <value> -> {json}\n";

    static bool removeMember(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr removeMemberName = "removeMember";
    static auto constexpr removeMemberDesc = "Removes the member at the specified key from the JSON document.\n"
            "Usage: |removeMember <key1> <key2> ... -> {json}\n";

    static bool assign(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr assignName = "assign";
    static auto constexpr assignDesc = "Assigns a value based on the result of an expression.\n"
        "Usage: |assign <context>:<key> <assign-operator> <expression> -> {json}\n"
        "The expression is evaluated in the current scope, and the result is assigned to the given key.\n"
        "The entire context is local, meaning self, other, global are all the same and refer to the current scope.\n";

    // TODO: Even though copy/move is implemented in DomainModule:JsonScope:SimpleData, having these as transformations as well could be useful
    //       instead of creating a dummy domain to parse a task, we can simply write |copy <source> <destination>
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_GENERAL_HPP
