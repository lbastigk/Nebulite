#ifndef MODULE_TRANSFORMATION_FILTER_HPP
#define MODULE_TRANSFORMATION_FILTER_HPP

//------------------------------------------
// Includes

// Standard library
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Interaction/Execution/FuncTree.hpp"
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {

class Filter final : public Base::TransformationModule {
public:
    explicit Filter(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override ;

    //------------------------------------------
    // Available Transformations

    static bool filterRegex(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr filterRegexName = "filterRegex";
    static auto constexpr filterRegexDesc = "Filters members in the current JSON array/object based on a regular expression pattern.\n"
        "For arrays, the member names are the indices as strings: [0], [1], [2], ...\n"
        "Wrap the pattern inside {!...} to avoid conflicts with piping character"
        "Usage: |filterRegex {!<pattern>} -> {filtered array}\n"
        "       |filterRegex <pattern>    -> {filtered array}\n";

    static bool filterGlob(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr filterGlobName = "filterGlob";
    static auto constexpr filterGlobDesc = "Filters members in the current JSON array/object based on a glob pattern.\n"
        "For arrays, the member names are the indices as strings: [0], [1], [2], ...\n"
        "Usage: |filterGlob <pattern> -> {filtered array}\n";

    static bool filterRegexValue(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr filterRegexValueName = "filterRegexValue";
    static auto constexpr filterRegexValueDesc = "Filters values in the current JSON array based on a regular expression pattern.\n"
        "Usage: |filterRegexValue {!<pattern>} -> {filtered array}\n"
        "       |filterRegexValue <pattern>    -> {filtered array}\n";

    static bool filterGlobValue(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr filterGlobValueName = "filterGlobValue";
    static auto constexpr filterGlobValueDesc = "Filters values in the current JSON array based on a glob pattern.\n"
        "Usage: |filterGlobValue <pattern> -> {filtered array}\n";

    static bool filterNulls(Data::JsonScope& jsonDoc);
    static auto constexpr filterOutNullsName = "filterNulls";
    static auto constexpr filterOutNullsDesc = "Filters out null values, empty objects, and empty arrays from the current JSON\n"
        "Usage: |filterNulls -> {filtered json}\n";

    static bool filterCustom(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr filterCustomName = "filterCustom";
    static auto constexpr filterCustomDesc = "Filters values in the current JSON array based on a custom filter expression\n"
        "Provide a filter expression without the evaluation key '$'!\n"
        "For example: ( gt({self:|length}, 3) )\n"
        "Usage: |filterCustom <expression> -> {filtered array}\n";

private:
    /**
     * @brief Custom JSON sort function
     * @param jsonDoc The scope to sort. Must be an array at scope root!
     * @param filter The filter function to use
     */
    static void arrayFilter(Data::JsonScope& jsonDoc, std::function<bool(Data::JsonScope&)> const& filter);

    /**
     * @brief List all member values of a JsonScope that are convertible to strings
     * @param jsonDoc The scope to list values from
     * @param rootKey The key of the scope to list values from. Must be an array
     * @return A vector of optional strings, where each element corresponds to a member value.
     */
    static std::vector<std::string> listMemberValues(Data::JsonScope const& jsonDoc, Data::ScopedKeyView const& rootKey);
};

} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_FILTER_HPP
