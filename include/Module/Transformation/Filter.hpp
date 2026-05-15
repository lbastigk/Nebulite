#ifndef MODULE_TRANSFORMATION_FILTER_HPP
#define MODULE_TRANSFORMATION_FILTER_HPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

class Filter final : public Base::TransformationModule {
public:
    explicit Filter(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override ;

    //------------------------------------------
    // Available Transformations

    static bool filterRegex(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr filterRegexName = "filterRegex";
    static auto constexpr filterRegexDesc = "Filters members in the current JSON array/object based on a regular expression pattern.\n"
        "For arrays, the member names are the indices as strings: [0], [1], [2], ...\n"
        "Wrap the pattern inside {!...} to avoid conflicts with piping character"
        "Usage: |filterRegex {!<pattern>} -> {filtered array}\n"
        "       |filterRegex <pattern>    -> {filtered array}\n";

    static bool filterGlob(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr filterGlobName = "filterGlob";
    static auto constexpr filterGlobDesc = "Filters members in the current JSON array/object based on a glob pattern.\n"
        "For arrays, the member names are the indices as strings: [0], [1], [2], ...\n"
        "Usage: |filterGlob <pattern> -> {filtered array}\n";

    static bool filterNulls(Data::JsonScope* jsonDoc);
    static auto constexpr filterOutNullsName = "filterNulls";
    static auto constexpr filterOutNullsDesc = "Filters out null values, empty objects, and empty arrays from the current JSON\n"
        "Usage: |filterNulls -> {filtered json}\n";

    static bool filterCustom(std::span<std::string const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr filterCustomName = "filterCustom";
    static auto constexpr filterCustomDesc = "Filters values in the current JSON array based on a custom filter expression\n"
        "Provide a filter expression without the evaluation key '$'!\n"
        "For example: ( gt({value:|length}, 3) )\n"
        "Usage: |filterCustom <expression> -> {filtered array}\n";

private:
    /**
     * @brief Custom JSON sort function
     * @param jsonDoc The scope to sort. Must be an array at scope root!
     * @param filter The filter function to use
     */
    static void arrayFilter(Data::JsonScope* jsonDoc, std::function<bool(Data::JsonScope&)> const& filter) {
        std::vector<Data::JSON> values;
        for (auto const idx : std::views::iota(std::size_t{0}, jsonDoc->memberSize(rootKey))) {
            auto const key = rootKey.addIndex(idx);
            auto doc = jsonDoc->getSubDoc(key);
            if (auto& scope = doc.shareManagedScopeBase(""); filter(scope)) {
                values.emplace_back(std::move(doc));
            }
        }
        for (auto [idx, value] : values | std::views::enumerate) {
            auto const key = rootKey.addIndex(static_cast<size_t>(idx));
            jsonDoc->setSubDoc(key, value);
        }
    }
};

} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_FILTER_HPP
