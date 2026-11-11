#ifndef NEBULITE_MODULE_TRANSFORMATION_REQUIREMENTS_HPP
#define NEBULITE_MODULE_TRANSFORMATION_REQUIREMENTS_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>

// Nebulite
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Utility/Args/FuncTree.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::Requirements
 * @brief Non-exiting assertion transformation functions
 * @details Instead of exiting, these transformations simply return false, indicating a transformation failure.
 *          Any user-defined error messages are forwarded to the Global capture.
 */
class Requirements final : public Base::TransformationModule {
public:
    explicit Requirements(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Categories

    static auto constexpr requireName = "require";
    static auto constexpr requireDesc = "Requirement transformations validate JSON values. If a requirement is not met, the transformation fails.";

    static auto constexpr requireTypeName = "require type";
    static auto constexpr requireTypeDesc = "Requirement transformations that validate JSON value types";

    static auto constexpr requireMatchName = "require match";
    static auto constexpr requireMatchDesc = "Requirement transformations that validate JSON string values with patterns.";

    static auto constexpr requireEqualsName = "require equals";
    static auto constexpr requireEqualsDesc = "Requirement transformations that validate JSON value equality.";

    //------------------------------------------
    // Available Transformations

    // [True/False]

    static bool requireTrue(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireTrueName = "require true";
    static auto constexpr requireTrueDesc = "Requires that the current JSON value is true.\n"
        "If the value is not true, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |require true -> {value,<Returns false if not true>}\n";

    static bool requireFalse(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireFalseName = "require false";
    static auto constexpr requireFalseDesc = "Requires that the current JSON value is false.\n"
        "If the value is not false, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |require false -> {value,<Returns false if not false>}\n";

    // [Existence]

    static bool requireNonEmpty(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireNonEmptyName = "require nonEmpty";
    static auto constexpr requireNonEmptyDesc = "Requires that the current JSON value is non-empty.\n"
        "If the value is empty, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireNonEmpty -> {value,<Returns false if empty>}\n";

    static bool requireEmpty(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireEmptyName = "require empty";
    static auto constexpr requireEmptyDesc = "Requires that the current JSON value is empty.\n"
        "If the value is not empty, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireEmpty -> {value,<Returns false if not empty>}\n";

    // [TYPE]

    static bool requireTypeObject(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireTypeObjectName = "require type object";
    static auto constexpr requireTypeObjectDesc = "Requires that the current JSON value is of type object.\n"
        "If the value is not an object, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeObject -> {value,<Returns false if not object>}\n";

    static bool requireTypeArray(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireTypeArrayName = "require type array";
    static auto constexpr requireTypeArrayDesc = "Requires that the current JSON value is of type array.\n"
        "If the value is not an array, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeArray -> {value,<Returns false if not array>}\n";

    static bool requireTypeBasicValue(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireTypeBasicValueName = "require type basicValue";
    static auto constexpr requireTypeBasicValueDesc = "Requires that the current JSON value is a basic value (not object or array or null).\n"
        "If the value is not a basic value, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeValue -> {value,<Returns false if not value>}\n";

    static bool requireTypeNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireTypeNumericName = "require type numeric";
    static auto constexpr requireTypeNumericDesc = "Requires a JSON number (not a boolean or string).\n"
        "If the value is not numeric, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeNumeric -> {value,<Returns false if not numeric>}\n";

    static bool requireTypeNumericOrNumericString(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireTypeNumericOrNumericStringName = "require type numericOrNumericString";
    static auto constexpr requireTypeNumericOrNumericStringDesc = "Requires a JSON number or a numeric string; booleans and boolean strings are rejected.\n"
        "If the value is not numeric or a string of a numeric, the transformation fails.\n"
        "Accepts an optional user-defined error message as additional arguments.\n"
        "Usage: |requireTypeNumericOrNumericString -> {value,<Returns false if not numeric or numeric string>}\n";

    // [MATCH]

    static bool requireMatchRegex(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireMatchRegexName = "require match regex";
    static auto constexpr requireMatchRegexDesc = "Requires that the current JSON string value matches a provided regular expression.\n"
        "If the pattern is missing or invalid, or the value does not match it, the transformation fails.\n"
        "Usage: |require match regex <regex> -> {value,<Returns false if no match>}\n";

    // [EQUALS]

    static bool requireEqualsString(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireEqualsStringName = "require equals string";
    static auto constexpr requireEqualsStringDesc = "Requires that the current JSON value is equal to the specified string.\n"
        "If the value is not equal to the specified string, the transformation fails. An omitted expected string is empty.\n"
        "Usage: |require equals string <expected> -> {value,<Returns false if not equal>}\n";

    static bool requireEqualsInt(Utility::Args::SegmentedStringView const& args, Data::JsonScope const& jsonDoc);
    static auto constexpr requireEqualsIntName = "require equals int";
    static auto constexpr requireEqualsIntDesc = "Requires that the current JSON value equals the specified integer.\n"
        "One expected integer is required; leading whitespace and a leading plus or minus are accepted, but boolean strings are not.\n"
        "Invalid, partially parsed, or out-of-range arguments and failed value conversions cause the transformation to fail.\n"
        "Usage: |require equals int <expected> -> {value,<Returns false if not equal>}\n";

};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_REQUIREMENTS_HPP
