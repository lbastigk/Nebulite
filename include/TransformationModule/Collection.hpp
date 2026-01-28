/**
 * @file Collection.hpp
 * @brief Class for collection transformation modules.
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

    // TODO: filter
    //       glob-patterns would be useful here
    //       but first, we need to implement object key retrieval in JSON class
    //bool filter(std::span<std::string const> const& args, JsonScope* jsonDoc);
    //static auto constexpr filterName;
    //static auto constexpr filterDesc;

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
};
} // namespace Nebulite::TransformationModule
#endif // NEBULITE_TRANSFORMATION_MODULE_COLLECTION_HPP
