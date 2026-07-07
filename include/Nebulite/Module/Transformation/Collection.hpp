#ifndef NEBULITE_MODULE_TRANSFORMATION_COLLECTION_HPP
#define NEBULITE_MODULE_TRANSFORMATION_COLLECTION_HPP

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
 * @class Nebulite::Module::Transformation::Collection
 * @brief Various transformations for applying functions to multiple entries, collecting data and collecting metadata.
 */
class Collection final : public Base::TransformationModule {
public:
    explicit Collection(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool map(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr mapName = "map";
    static auto constexpr mapDesc = "Applies a mapping function to each element in the array of the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |map <function> -> {array}\n";

    static bool get(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
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

    static bool bundleToArray(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr bundleToArrayName = "bundleToArray";
    static auto constexpr bundleToArrayDesc = "Gathers all members from the provided keys into an array.\n"
        "Usage: |bundleToArray <key1> <key2> ... -> {array}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_COLLECTION_HPP
