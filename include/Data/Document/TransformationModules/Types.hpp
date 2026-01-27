/**
 * @file Types.hpp
 * @brief Class for types transformation modules.
 */

#ifndef NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_TYPES_HPP
#define NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_TYPES_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::Data::TransformationModules {

class Types final : public TransformationModule {
public:
    explicit Types(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool typeAsNumber(Core::JsonScope* jsonDoc);
    static auto constexpr typeAsNumberName = "typeAsNumber";
    static auto constexpr typeAsNumberDesc = "Converts the current JSON type value to a number.\n"
        "Usage: |typeAsNumber -> {number}"
        "where the number reflects the enum value KeyType.\n";

    static bool typeAsString(Core::JsonScope* jsonDoc);
    static auto constexpr typeAsStringName = "typeAsString";
    static auto constexpr typeAsStringDesc = "Converts the current JSON type value to a string.\n"
        "Usage: |typeAsString -> {value,array,object}\n";

    // TODO: exists, isType <name>

    static bool serialize(Core::JsonScope* jsonDoc);
    static auto constexpr serializeName = "serialize";
    static auto constexpr serializeDesc = "Serializes the current JSON value to a string.\n"
        "Usage: |serialize -> {string}\n";

    static bool deserialize(Core::JsonScope* jsonDoc);
    static auto constexpr deserializeName = "deserialize";
    static auto constexpr deserializeDesc = "Deserializes the current JSON string value.\n"
            "Usage: |deserialize -> {value}\n";
};
} // namespace Nebulite::Data::TransformationModules
#endif // NEBULITE_DATA_DOCUMENT_TRANSFORMATION_MODULES_TYPES_HPP
