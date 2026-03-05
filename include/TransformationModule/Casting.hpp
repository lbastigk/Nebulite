/**
 * @file Casting.hpp
 * @brief Class for casting transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_CASTING_HPP
#define NEBULITE_TRANSFORMATION_MODULE_CASTING_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Casting final : public Data::TransformationModule {
public:
    explicit Casting(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScopeBase*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool toInt(Data::JsonScopeBase* jsonDoc);
    static auto constexpr toIntName = "toInt";
    static auto constexpr toIntDesc = "Casts the current JSON value to an integer.\n"
        "Usage: |toInt -> {value:int}\n"
        "Non-numeric values default to 0, but fails if the value is null.\n";

    static bool toString(Data::JsonScopeBase* jsonDoc);
    static auto constexpr toStringName = "toString";
    static auto constexpr toStringDesc = "Converts the current JSON value to a string.\n"
        "Defaults to an empty string if no conversion is possible, but fails if the value is null.\n"
        "Usage: |toString -> {value:string}\n";

    static bool toBool(Data::JsonScopeBase* jsonDoc);
    static auto constexpr toBoolName = "toBool";
    static auto constexpr toBoolDesc = "Converts the current JSON value to a boolean.\n"
        "Usage: |toBool -> {value:bool}\n"
        "'true' values: true, 1, '1', 'true', 'yes', 'on' (case-insensitive)\n"
        "'false' values: false, 0, '0', 'false', 'no', 'off' (case-insensitive)\n"
        "Any other value defaults to false, but fails if the value is null.\n";

    static bool toDouble(Data::JsonScopeBase* jsonDoc);
    static auto constexpr toDoubleName = "toDouble";
    static auto constexpr toDoubleDesc = "Converts the current JSON value to a double.\n"
        "Usage: |toDouble -> {value:double}\n"
        "Non-numeric values default to 0.0, but fails if the value is null.\n";

    static bool toBoolString(Data::JsonScopeBase* jsonDoc);
    static auto constexpr toBoolStringName = "toBoolString";
    static auto constexpr toBoolStringDesc = "Converts the current JSON value to a boolean string.\n"
        "Usage: |toBoolString -> {value:string}\n"
        "Either 'true' or 'false'\n";

    static bool roundUp(Data::JsonScopeBase* jsonDoc);
    static auto constexpr roundUpName = "roundUp";
    static auto constexpr roundUpDesc = "Rounds the current JSON numeric value up to the nearest integer.\n"
        "Usage: |roundUp -> {value:int}\n"
        "Non-numeric values default to 0.\n";

    static bool roundDown(Data::JsonScopeBase* jsonDoc);
    static auto constexpr roundDownName = "roundDown";
    static auto constexpr roundDownDesc = "Rounds the current JSON numeric value down to the nearest integer.\n"
        "Usage: |roundDown -> {value:int}\n"
    "Non-numeric values default to 0. Fails if the value is null.\n";

    static bool round(Data::JsonScopeBase* jsonDoc);
    static auto constexpr roundName = "round";
    static auto constexpr roundDesc = "Rounds the current JSON numeric value to the nearest integer.\n"
        "Usage: |round -> {value:int}\n"
        "Non-numeric values default to 0. Fails if the value is null.\n";
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_CASTING_HPP
