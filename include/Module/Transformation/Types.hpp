/**
 * @file Types.hpp
 * @brief Class for types transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_TYPES_HPP
#define NEBULITE_TRANSFORMATION_MODULE_TYPES_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Types final : public Data::TransformationModule {
public:
    explicit Types(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool typeAsNumber(Data::JsonScope* jsonDoc);
    static auto constexpr typeAsNumberName = "typeAsNumber";
    static auto constexpr typeAsNumberDesc = "Converts the current JSON type value to a number.\n"
        "Usage: |typeAsNumber -> {number}"
        "where the number reflects the enum value KeyType.\n";

    static bool typeAsSimpleString(Data::JsonScope* jsonDoc);
    static auto constexpr typeAsSimpleStringName = "typeAsSimpleString";
    static auto constexpr typeAsSimpleStringDesc = "Converts the current JSON type value to a string.\n"
        "Usage: |typeAsString -> {value,array,object}\n";

    static bool typeAsString(Data::JsonScope* jsonDoc);
    static auto constexpr typeAsStringName = "typeAsString";
    static auto constexpr typeAsStringDesc = "Converts the current JSON type value to a string with metadata.\n"
        "Usage: |typeAsString -> {value:metadata,array:metadata,object:metadata}\n";

    static bool serialize(Data::JsonScope* jsonDoc);
    static auto constexpr serializeName = "serialize";
    static auto constexpr serializeDesc = "Serializes the current JSON value to a string.\n"
        "Usage: |serialize -> {string}\n";

    static bool deserialize(Data::JsonScope* jsonDoc);
    static auto constexpr deserializeName = "deserialize";
    static auto constexpr deserializeDesc = "Deserializes the current JSON string value stored in root.\n"
        "Usage: |deserialize -> {value}\n";

    static bool exists(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr existsName = "exists";
    static auto constexpr existsDesc = "Checks if a specified key exists in the current JSON object.\n"
        "If no key is provided, checks if the current JSON value is not null.\n"
        "Usage: |exists  [key] -> {bool}\n";
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_TYPES_HPP
