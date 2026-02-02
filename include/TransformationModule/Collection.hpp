/**
 * @file Collection.hpp
 * @brief Class for collection transformation functions.
 */

#ifndef NEBULITE_TRANSFORMATION_MODULE_COLLECTION_HPP
#define NEBULITE_TRANSFORMATION_MODULE_COLLECTION_HPP

#include "Data/Document/TransformationModule.hpp"

namespace Nebulite::TransformationModule {

class Collection final : public Data::TransformationModule {
public:
    explicit Collection(std::shared_ptr<Interaction::Execution::FuncTree<bool, Core::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    bool map(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) const ; // Map needs to reparse, so it's a member function
    static auto constexpr mapName = "map";
    static auto constexpr mapDesc = "Applies a mapping function to each element in the array of the current JSON value.\n"
        "If the current value is not an array, it is first wrapped into a single-element array.\n"
        "Usage: |map <function> -> {array}\n";

    static bool get(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr getName = "get";
    static auto constexpr getDesc = "Gets the value at the specified key from the current JSON object.\n"
        "Usage: |get <key> -> {value}\n";

    static bool getMultiple(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr getMultipleName = "getMultiple";
    static auto constexpr getMultipleDesc = "Gets multiple values at the specified keys from the current JSON object.\n"
        "Usage: |getMultiple <key1> <key2> ... -> {array of values}\n";

    // TODO: filter-regex

    static bool filterGlob(std::span<std::string const> const& args, Core::JsonScope* jsonDoc);
    static auto constexpr filterGlobName = "filterGlob";
    static auto constexpr filterGlobDesc = "Filters members in the current JSON array/object based on a glob pattern.\n"
        "For arrays, the member names are the indices as strings: [0], [1], [2], ...\n"
        "Usage: |filterGlob <pattern> -> {filtered array}\n";

    static bool filterNulls(Core::JsonScope* jsonDoc);
    static auto constexpr filterOutNullsName = "filterNulls";
    static auto constexpr filterOutNullsDesc = "Filters out null values, empty objects, and empty arrays from the current JSON\n"
        "Usage: |filterNulls -> {filtered json}\n";

    static bool listMembers(Core::JsonScope* jsonDoc);
    static auto constexpr listKeysName = "listMembers";
    static auto constexpr listKeysDesc = "Lists all members of the current JSON object as an array.\n"
        "If the current value is an array, it lists the indices as strings.\n"
        "Usage: |listKeys -> {array of keys}\n";
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_COLLECTION_HPP
