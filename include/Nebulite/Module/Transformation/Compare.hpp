#ifndef NEBULITE_MODULE_TRANSFORMATION_COMPARE_HPP
#define NEBULITE_MODULE_TRANSFORMATION_COMPARE_HPP

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
 * @class Nebulite::Module::Transformation::Compare
 * @brief Various transformations for comparing JSON values.
 */
class Compare final : public Base::TransformationModule {
public:
    explicit Compare(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool gt(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr gtName = "gt";
    static auto constexpr gtDesc = "Checks if the current JSON value is greater than the specified value.\n"
        "Usage: |gt <value> -> {bool}\n";

    static bool geq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr geqName = "geq";
    static auto constexpr geqDesc = "Checks if the current JSON value is greater than or equal to the specified value.\n"
        "Usage: |geq <value> -> {bool}\n";

    static bool lt(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr ltName = "lt";
    static auto constexpr ltDesc = "Checks if the current JSON value is less than the specified value.\n"
        "Usage: |lt <value> -> {bool}\n";

    static bool leq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr leqName = "leq";
    static auto constexpr leqDesc = "Checks if the current JSON value is less than or equal to the specified value.\n"
        "Usage: |leq <value> -> {bool}\n";

    static bool eq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr eqName = "eq";
    static auto constexpr eqDesc = "Checks if the current JSON value is equal to the specified value.\n"
        "Usage: |eq <value> -> {bool}\n";

    static bool neq(std::span<std::string_view const> const& args, Data::JsonScope& jsonDoc);
    static auto constexpr neqName = "neq";
    static auto constexpr neqDesc = "Checks if the current JSON value is not equal to the specified value.\n"
        "Usage: |neq <value> -> {bool}\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_COMPARE_HPP
