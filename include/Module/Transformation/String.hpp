/**
 * @file String.hpp
 * @brief Class for string transformation functions.
 */

#ifndef MODULE_TRANSFORMATION_STRING_HPP
#define MODULE_TRANSFORMATION_STRING_HPP

//------------------------------------------
// Includes

// Standard library
#include <memory>
#include <span>

// Nebulite
#include "Interaction/Execution/FuncTree.hpp"
#include "Module/Base/TransformationModule.hpp"

//------------------------------------------
// Forward declarations

namespace Nebulite::Data {
class JsonScope;
} // namespace Nebulite::Data

//------------------------------------------
namespace Nebulite::Module::Transformation {
/**
 * @class Nebulite::Module::Transformation::String
 * @brief Various string conversion transformation functions
 */
class String final : public Base::TransformationModule {
public:
    explicit String(std::shared_ptr<Interaction::Execution::FuncTree<bool, Data::JsonScope*>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool strLen(Data::JsonScope* jsonDoc);
    static auto constexpr strLenName = "strLen";
    static auto constexpr strLenDesc = "Calculates the length of the current JSON string value.\n"
        "Usage: |strLen -> {int}\n";

    static bool capitalize(Data::JsonScope* jsonDoc);
    static auto constexpr capitalizeName = "capitalize";
    static auto constexpr capitalizeDesc = "Capitalizes the current JSON string.\n"
        "Usage: |capitalize {string} -> {string}\n"
        "Only capitalizes the first character!\n";

    static bool toUpper(Data::JsonScope* jsonDoc);
    static auto constexpr toUpperName = "toUpper";
    static auto constexpr toUpperDesc = "Converts the current JSON string value to uppercase.\n"
        "Usage: |toUpper -> {string}\n";

    static bool toLower(Data::JsonScope* jsonDoc);
    static auto constexpr toLowerName = "toLower";
    static auto constexpr toLowerDesc = "Converts the current JSON string value to lowercase.\n"
        "Usage: |toLower -> {string}\n";

    static bool lPad(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr lPadName = "lPad";
    static auto constexpr lPadDesc = "Pads the current JSON string value on the left with a specified character until it reaches a specified total length.\n"
        "Usage: |lPad {totalLength} {padChar} -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: Character to use for padding (if not provided, defaults to space)\n";

    static bool rPad(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr rPadName = "rPad";
    static auto constexpr rPadDesc = "Pads the current JSON string value on the right with a specified character until it reaches a specified total length.\n"
        "Usage: |rPad {totalLength} {padChar} -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: Character to use for padding (if not provided, defaults to space)\n";

    static bool lPadNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr lPadNumericName = "lPadNumeric";
    static auto constexpr lPadNumericDesc = "Pads the current JSON numeric string value on the left with a specified character until it reaches a specified total length.\n"
        "If the value is not numeric, it is not modified.\n"
        "Usage: |lPadNumeric {totalLength} {padChar} -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: Character to use for padding (if not provided, defaults to space)\n";

    static bool rPadNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr rPadNumericName = "rPadNumeric";
    static auto constexpr rPadNumericDesc = "Pads the current JSON numeric string value on the right with a specified character until it reaches a specified total length.\n"
        "If the value is not numeric, it is not modified.\n"
        "Usage: |lPadNumeric {totalLength} {padChar} -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: Character to use for padding (if not provided, defaults to space)\n";

    static bool lPadNonNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr lPadNonNumericName = "lPadNonNumeric";
    static auto constexpr lPadNonNumericDesc = "Pads the current JSON non-numeric string value on the left with a specified character until it reaches a specified total length.\n"
        "If the value is numeric, it is not modified.\n"
        "Usage: |lPadNumeric {totalLength} {padChar} -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: Character to use for padding (if not provided, defaults to space)\n";

    static bool rPadNonNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr rPadNonNumericName = "rPadNonNumeric";
    static auto constexpr rPadNonNumericDesc = "Pads the current JSON non-numeric string value on the right with a specified character until it reaches a specified total length.\n"
        "If the value is numeric, it is not modified.\n"
        "Usage: |lPadNumeric {totalLength} {padChar} -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: Character to use for padding (if not provided, defaults to space)\n";

    static bool strip(Data::JsonScope* jsonDoc);
    static auto constexpr trimName = "strip";
    static auto constexpr trimDesc = "Strips whitespace from both ends of the current JSON string value.\n"
        "Usage: |trim -> {string}\n";

    static bool lStrip(Data::JsonScope* jsonDoc);
    static auto constexpr lStripName = "lStrip";
    static auto constexpr lStripDesc = "Strips whitespace from the left end of the current JSON string value.\n"
        "Usage: |lStrip -> {string}\n";

    static bool rStrip(Data::JsonScope* jsonDoc);
    static auto constexpr rStripName = "rStrip";
    static auto constexpr rStripDesc = "Strips whitespace from the right end of the current JSON string value.\n"
            "Usage: |rStrip -> {string}\n";

    static bool substring(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr substringName = "substring";
    static auto constexpr substringDesc = "Extracts a substring from the current JSON string value.\n"
        "Usage: |substring {start} [length] -> {string}\n"
        "{start}: Starting index (0-based)\n"
        "[length]: Length of the substring. Optional\n";

    static bool replace(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr replaceName = "replace";
    static auto constexpr replaceDesc = "Replaces all occurrences of a target substring with a replacement substring in the current JSON string value.\n"
        "Usage: |replace {target} {replacement} -> {string}\n"
        "{target}: Substring to be replaced\n"
        "{replacement}: Substring to replace with\n"
        "If only {target} is provided, it will be replaced with an empty string.\n"
        "If the target or replacement strings contain spaces, use an arrow '->' to separate them:"
        "|replace {target} -> {replacement}\n";

    static bool strCountAppearance(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr strCountAppearanceName = "strCountAppearance";
    static auto constexpr strCountAppearanceDesc = "Counts the number of occurrences of a specified substring in the current JSON string value.\n"
        "Usage: |strCountAppearance {substring} -> {number}\n"
        "Counts whitespaces if no substring is given\n"
        "{substring}: Substring to count\n";

    static bool split(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr splitName = "split";
    static auto constexpr splitDesc = "Splits the current JSON string value into an array of substrings based on a specified character delimiter.\n"
        "Usage: |split {delimiter} -> {array}\n"
        "{delimiter}: Delimiter to split the string by. If left empty, it splits by whitespace.\n";

    //------------------------------------------
    // strcompare

    static auto constexpr strcompareName = "strCompare";
    static auto constexpr strcompareDesc = "Functions for comparing string values.\n"
        "Usage: |strCompare {comparisonType} {string} -> {bool}\n"
        "{comparisonType}: Type of comparison\n"
        "{string}: String to compare with\n";

    static bool strcompareEquals(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr strcompareEqualsName = "strCompare equals";
    static auto constexpr strcompareEqualsDesc = "Compares the current JSON string value with a specified string for equality.\n"
        "Usage: |strCompare equals {string} -> {bool}\n"
        "Checks if the string is empty if no argument is given\n"
        "{string}: String to compare with\n";

    static bool strcompareContains(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr strcompareContainsName = "strCompare contains";
    static auto constexpr strcompareContainsDesc = "Checks if the current JSON string value contains a specified substring.\n"
        "Usage: |strCompare contains {string} -> {bool}\n"
        "Checks for whitespaces if no substring is given\n"
        "{string}: Substring to check for\n";

    static bool strcompareStartsWith(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr strcompareStartsWithName = "strCompare startsWith";
    static auto constexpr strcompareStartsWithDesc = "Checks if the current JSON string value starts with a specified substring.\n"
        "Usage: |strCompare startsWith {string} -> {bool}\n"
        "{string}: Substring to check for. If left empty, it checks for a whitespace.\n";

    static bool strcompareEndsWith(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc);
    static auto constexpr strcompareEndsWithName = "strCompare endsWith";
    static auto constexpr strcompareEndsWithDesc = "Checks if the current JSON string value ends with a specified substring.\n"
        "Usage: |strCompare endsWith {string} -> {bool}\n"
        "{string}: Substring to check for. If left empty, it checks for a whitespace.\n";
};
} // namespace Nebulite::Module::Transformation
#endif // MODULE_TRANSFORMATION_STRING_HPP
