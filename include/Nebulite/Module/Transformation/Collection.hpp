#ifndef NEBULITE_MODULE_TRANSFORMATION_COLLECTION_HPP
#define NEBULITE_MODULE_TRANSFORMATION_COLLECTION_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>
#include <string_view>

// Nebulite
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Collection
 * @brief Various transformations for applying functions to multiple entries, collecting data and collecting metadata.
 */
class Collection final : public Base::TransformationModule {
public:
    explicit Collection(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool map(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr mapName = "map";
    static auto constexpr mapDesc = "Applies a mapping function to each element in the array of the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |map <function> -> {array}\n";

    static bool get(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr getName = "get";
    static auto constexpr getDesc = "Gets the value at the specified key from the current JSON object.\n"
        "Usage: |get <key> -> {value}\n";

    static bool listMembers(Data::JsonScope& jsonDoc);
    static auto constexpr listMembersName = "listMembers";
    static auto constexpr listMembersDesc = "Lists all members of the current JSON object as an array.\n"
        "If the current value is an array, it lists the indices as strings.\n"
        "Usage: |listKeys -> {array of keys}\n";

    static bool listMembersAndValues(Data::JsonScope& jsonDoc);
    static auto constexpr listMembersAndValuesName = "listMembersAndValues";
    static auto constexpr listMembersAndValuesDesc = "Lists all members of the current JSON object as an array of {key, value} objects.\n"
        "If the current value is an array, it lists the indices as strings.\n"
        "Usage: |listMembersAndValues -> {array of {key, value} objects}\n";

    static bool enumerate(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr enumerateName = "enumerate";
    static auto constexpr enumerateDesc = "Enumerates the array in the current JSON value.\n"
        "Creates another array for each element, where [0] is the index and [1] is the value.\n"
        "Recommended over enumerateInline if the values stored are not objects.\n"
        "Usage: |enumerate -> {array}\n"
        "Where indexKey is the key of each array element to populate with the index of the element in the array.\n";

    static bool enumerateInline(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr enumerateInlineName = "enumerateInline";
    static auto constexpr enumerateInlineDesc = "Enumerates the array in the current JSON value using a provided key.\n"
        "Recommended over enumerate if the values stored are objects.\n"
        "Usage: |enumerateInline <indexKey> -> {array}\n"
        "Where indexKey is the key of each array element to populate with the index of the element in the array.\n";

    static bool bundle(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr bundleName = "bundle";
    static auto constexpr bundleDesc = "Gathers all members from the provided keys into an array.\n"
        "Inserts empty objects for missing values in the object.\n"
        "Usage: |bundle <key1> <key2> ... -> {array}\n";

    static bool bind(std::span<std::string_view const> args, Data::JsonScope& jsonDoc);
    static auto constexpr bindName = "bind";
    static auto constexpr bindDesc = "Binds the elements of an array to the provided keys.\n"
        "think of C++ structured bindings: auto [key1, key2, key3] = arr;\n"
        "Fails if the given scope is not an array or if no keys are provided.\n"
        "Inserts empty objects for missing values in the array.\n"
        "Usage: |bind <key1> <key2> ... -> {object}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_COLLECTION_HPP
