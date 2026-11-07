#ifndef NEBULITE_MODULE_TRANSFORMATION_STRING_HPP
#define NEBULITE_MODULE_TRANSFORMATION_STRING_HPP

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
 * @class Nebulite::Module::Transformation::String
 * @brief Various string conversion transformation functions
 */
class String final : public Base::TransformationModule {
public:
    explicit String(std::shared_ptr<Utility::Args::FuncTree<bool, Data::JsonScope&>> const& funcTree)
        : TransformationModule(funcTree) {}

    void bindTransformations() override;

    //------------------------------------------
    // Available Transformations

    static bool strLen(Data::JsonScope& jsonDoc);
    static auto constexpr strLenName = "strLen";
    static auto constexpr strLenDesc = "Calculates the length of the current JSON string value.\n"
        "Length is measured in bytes, not UTF-8 characters. Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strLen -> {int}\n";

    static bool capitalize(Data::JsonScope& jsonDoc);
    static auto constexpr capitalizeName = "capitalize";
    static auto constexpr capitalizeDesc = "Capitalizes the current JSON string.\n"
        "Usage: |capitalize {string} -> {string}\n"
        "Uppercases the first byte and lowercases the rest; this is not Unicode-aware. Failed string conversions use an empty string.\n";

    static bool toUpper(Data::JsonScope& jsonDoc);
    static auto constexpr toUpperName = "toUpper";
    static auto constexpr toUpperDesc = "Converts the current JSON string value to uppercase.\n"
        "Operates on bytes, not Unicode characters. Failed string conversions use an empty string.\n"
        "Usage: |toUpper -> {string}\n";

    static bool toLower(Data::JsonScope& jsonDoc);
    static auto constexpr toLowerName = "toLower";
    static auto constexpr toLowerDesc = "Converts the current JSON string value to lowercase.\n"
        "Operates on bytes, not Unicode characters. Failed string conversions use an empty string.\n"
        "Usage: |toLower -> {string}\n";

    static bool lPad(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr lPadName = "lPad";
    static auto constexpr lPadDesc = "Pads the current JSON string value on the left with a specified character until it reaches a specified total length.\n"
        "Usage: |lPad {totalLength} [padChar] -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: One byte; defaults to space if omitted. Empty or multi-byte arguments fail.\n"
        "Length is measured in bytes. Values are converted to strings; failed conversions use an empty string.\n";

    static bool rPad(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr rPadName = "rPad";
    static auto constexpr rPadDesc = "Pads the current JSON string value on the right with a specified character until it reaches a specified total length.\n"
        "Usage: |rPad {totalLength} [padChar] -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "{padChar}: One byte; defaults to space if omitted. Empty or multi-byte arguments fail.\n"
        "Length is measured in bytes. Values are converted to strings; failed conversions use an empty string.\n";

    static bool lPadNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr lPadNumericName = "lPadNumeric";
    static auto constexpr lPadNumericDesc = "Pads the current JSON numeric string value on the left with a specified character until it reaches a specified total length.\n"
        "If the value is not numeric, it is not modified.\n"
        "Usage: |lPadNumeric {totalLength} [padChar] -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "[padChar]: One byte; defaults to space if omitted. Empty or multi-byte arguments fail.\n"
        "Arguments are validated even when no padding is applied. Length is in bytes; failed string conversions use an empty string.\n";

    static bool rPadNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr rPadNumericName = "rPadNumeric";
    static auto constexpr rPadNumericDesc = "Pads the current JSON numeric string value on the right with a specified character until it reaches a specified total length.\n"
        "If the value is not numeric, it is not modified.\n"
        "Usage: |rPadNumeric {totalLength} [padChar] -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "[padChar]: One byte; defaults to space if omitted. Empty or multi-byte arguments fail.\n"
        "Arguments are validated even when no padding is applied. Length is in bytes; failed string conversions use an empty string.\n";

    static bool lPadNonNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr lPadNonNumericName = "lPadNonNumeric";
    static auto constexpr lPadNonNumericDesc = "Pads the current JSON non-numeric string value on the left with a specified character until it reaches a specified total length.\n"
        "If the value is numeric, it is not modified.\n"
        "Usage: |lPadNonNumeric {totalLength} [padChar] -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "[padChar]: One byte; defaults to space if omitted. Empty or multi-byte arguments fail.\n"
        "Arguments are validated even when no padding is applied. Length is in bytes; failed string conversions use an empty string.\n";

    static bool rPadNonNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr rPadNonNumericName = "rPadNonNumeric";
    static auto constexpr rPadNonNumericDesc = "Pads the current JSON non-numeric string value on the right with a specified character until it reaches a specified total length.\n"
        "If the value is numeric, it is not modified.\n"
        "Usage: |rPadNonNumeric {totalLength} [padChar] -> {string}\n"
        "{totalLength}: Desired total length of the resulting string (including original string and padding)\n"
        "[padChar]: One byte; defaults to space if omitted. Empty or multi-byte arguments fail.\n"
        "Arguments are validated even when no padding is applied. Length is in bytes; failed string conversions use an empty string.\n";

    static bool strip(Data::JsonScope& jsonDoc);
    static auto constexpr trimName = "strip";
    static auto constexpr trimDesc = "Strips whitespace from both ends of the current JSON string value.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strip -> {string}\n";

    static bool lStrip(Data::JsonScope& jsonDoc);
    static auto constexpr lStripName = "lStrip";
    static auto constexpr lStripDesc = "Strips whitespace from the left end of the current JSON string value.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |lStrip -> {string}\n";

    static bool rStrip(Data::JsonScope& jsonDoc);
    static auto constexpr rStripName = "rStrip";
    static auto constexpr rStripDesc = "Strips whitespace from the right end of the current JSON string value.\n"
            "Values are converted to strings; failed conversions use an empty string.\n"
            "Usage: |rStrip -> {string}\n";

    static bool substring(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr substringName = "substring";
    static auto constexpr substringDesc = "Extracts a substring from the current JSON string value.\n"
        "Usage: |substring {start} [length] -> {string}\n"
        "{start}: Non-negative byte index (0-based); out-of-range starts yield an empty string.\n"
        "[length]: Non-negative byte count; omitted means to the end and oversized counts are clamped.\n"
        "All arguments are validated before slicing. UTF-8 characters may be split; failed string conversions use an empty string.\n";

    static bool replace(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr replaceName = "replace";
    static auto constexpr replaceDesc = "Replaces all occurrences of a target substring with a replacement substring in the current JSON string value.\n"
        "Usage: |replace {target} {replacement} -> {string}\n"
        "{target}: Substring to be replaced\n"
        "{replacement}: Substring to replace with\n"
        "If only {target} is provided, it will be replaced with an empty string.\n"
        "The target must not be empty. Matches are non-overlapping and replaced from left to right.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "If the target or replacement strings contain spaces, use an arrow '->' to separate them:"
        "|replace {target} -> {replacement}\n";

    static bool strCountAppearance(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr strCountAppearanceName = "strCountAppearance";
    static auto constexpr strCountAppearanceDesc = "Counts the number of occurrences of a specified substring in the current JSON string value.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strCountAppearance {substring} -> {number}\n"
        "Counts whitespaces if no substring is given\n"
        "{substring}: Substring to count\n";

    static bool split(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr splitName = "split";
    static auto constexpr splitDesc = "Splits the current JSON string value into an array of substrings based on a specified character delimiter.\n"
        "Usage: |split {delimiter} -> {array}\n"
        "{delimiter}: One-byte delimiter; defaults to a space if omitted. Empty or multi-byte arguments fail.\n"
        "Keeps empty tokens between or after delimiters; empty input yields an empty array.\n"
        "Values are converted to strings; failed conversions use an empty string.\n";

    static bool splitNewlines(Data::JsonScope& jsonDoc);
    static auto constexpr splitNewlinesName = "splitNewlines";
    static auto constexpr splitNewlinesDesc = "Splits the current JSON string value into an array of substrings at newline characters.\n"
        "Treats CRLF as one newline; preserves empty tokens and standalone CR. Empty input yields an empty array.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |splitNewlines -> {array}\n";

    //------------------------------------------
    // strcompare

    static auto constexpr strcompareName = "strCompare";
    static auto constexpr strcompareDesc = "Functions for comparing string values.\n"
        "Usage: |strCompare {comparisonType} {string} -> {bool}\n"
        "{comparisonType}: Type of comparison\n"
        "{string}: String to compare with\n";

    static bool strcompareMembers(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr strcompareMembersName = "strCompare members";
    static auto constexpr strcompareMembersDesc = "Compare multiple member string values.\n"
        "Usage: |strCompareMembers {key1} {key2} ... {keyN} -> {bool}\n"
        "Checks if all specified member keys have the same string value; failed conversions make the comparison false.\n";

    static bool strcompareEquals(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr strcompareEqualsName = "strCompare equals";
    static auto constexpr strcompareEqualsDesc = "Compares the current JSON string value with a specified string for equality.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strCompare equals {string} -> {bool}\n"
        "Checks if the string is empty if no argument is given\n"
        "{string}: String to compare with\n";

    static bool strcompareContains(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr strcompareContainsName = "strCompare contains";
    static auto constexpr strcompareContainsDesc = "Checks if the current JSON string value contains a specified substring.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strCompare contains {string} -> {bool}\n"
        "Checks for whitespaces if no substring is given\n"
        "{string}: Substring to check for\n";

    static bool strcompareStartsWith(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr strcompareStartsWithName = "strCompare startsWith";
    static auto constexpr strcompareStartsWithDesc = "Checks if the current JSON string value starts with a specified substring.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strCompare startsWith {string} -> {bool}\n"
        "{string}: Substring to check for. If left empty, it checks for a whitespace.\n";

    static bool strcompareEndsWith(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc);
    static auto constexpr strcompareEndsWithName = "strCompare endsWith";
    static auto constexpr strcompareEndsWithDesc = "Checks if the current JSON string value ends with a specified substring.\n"
        "Values are converted to strings; failed conversions use an empty string.\n"
        "Usage: |strCompare endsWith {string} -> {bool}\n"
        "{string}: Substring to check for. If left empty, it checks for a whitespace.\n";
};
} // namespace Nebulite::Module::Transformation
#endif // NEBULITE_MODULE_TRANSFORMATION_STRING_HPP
