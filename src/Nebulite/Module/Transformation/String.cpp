//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error> // NOLINT

// Nebulite
#include "Nebulite/Data/Document/JsonScope.hpp"
#include "Nebulite/Module/Base/TransformationModule.hpp"
#include "Nebulite/Module/Transformation/String.hpp"
#include "Nebulite/Utility/Args/SegmentedStringView.hpp"
#include "Nebulite/Utility/Ranges.hpp"
#include "Nebulite/Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

namespace {

std::optional<std::size_t> parseSize(std::string_view const value) {
    if (value.empty()) {
        return std::nullopt;
    }
    std::size_t result{};
    auto const [end, error] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (error != std::errc{} || end != value.data() + value.size()) {
        return std::nullopt;
    }
    return result;
}

struct PadArguments {
    std::size_t size;
    char character;
};

std::optional<PadArguments> parsePadArguments(Utility::Args::SegmentedStringView const& args) {
    if (args.size() < 2 || args.size() > 3 || (args.size() == 3 && args[2].size() != 1)) {
        return std::nullopt;
    }
    auto const size = parseSize(args[1]);
    if (!size) {
        return std::nullopt;
    }
    return PadArguments{.size = *size, .character = args.size() == 3 ? args[2][0] : ' '};
}

bool pad(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc, bool const left,
         std::optional<bool> const numericCondition = std::nullopt) {
    auto const options = parsePadArguments(args);
    if (!options) {
        return false;
    }
    auto str = jsonDoc.get<std::string>(Base::TransformationModule::rootKey).value_or("");
    if (numericCondition && Utility::StringHandler::isNumber(str) != *numericCondition) {
        return true;
    }
    if (str.size() >= options->size) {
        return true;
    }
    std::string const padding(options->size - str.size(), options->character);
    jsonDoc.set(Base::TransformationModule::rootKey, left ? padding + str : str + padding);
    return true;
}

} // namespace

void String::bindTransformations() {
    bindTransformation(&strLen, strLenName, strLenDesc);
    bindTransformation(&capitalize, capitalizeName, capitalizeDesc);
    bindTransformation(&toUpper, toUpperName, toUpperDesc);
    bindTransformation(&toLower, toLowerName, toLowerDesc);
    bindTransformation(&lPad, lPadName, lPadDesc);
    bindTransformation(&rPad, rPadName, rPadDesc);
    bindTransformation(&lPadNumeric, lPadNumericName, lPadNumericDesc);
    bindTransformation(&rPadNumeric, rPadNumericName, rPadNumericDesc);
    bindTransformation(&lPadNonNumeric, lPadNonNumericName, lPadNonNumericDesc);
    bindTransformation(&rPadNonNumeric, rPadNonNumericName, rPadNonNumericDesc);
    bindTransformation(&strip, trimName, trimDesc);
    bindTransformation(&lStrip, lStripName, lStripDesc);
    bindTransformation(&rStrip, rStripName, rStripDesc);
    bindTransformation(&substring, substringName, substringDesc);
    bindTransformation(&replace, replaceName, replaceDesc);
    bindTransformation(&strCountAppearance, strCountAppearanceName, strCountAppearanceDesc);
    bindTransformation(&split, splitName, splitDesc);
    bindTransformation(&splitNewlines, splitNewlinesName, splitNewlinesDesc);

    bindCategory(strcompareName, strcompareDesc);
    bindTransformation(&strcompareMembers, strcompareMembersName, strcompareMembersDesc);
    bindTransformation(&strcompareEquals, strcompareEqualsName, strcompareEqualsDesc);
    bindTransformation(&strcompareContains, strcompareContainsName, strcompareContainsDesc);
    bindTransformation(&strcompareStartsWith, strcompareStartsWithName, strcompareStartsWithDesc);
    bindTransformation(&strcompareEndsWith, strcompareEndsWithName, strcompareEndsWithDesc);
}

bool String::strLen(Data::JsonScope& jsonDoc){
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.set(rootKey, str.size());
    return true;
}

bool String::capitalize(Data::JsonScope& jsonDoc){
    auto str = jsonDoc.get<std::string>(rootKey).value_or("");
    if (!str.empty()) {
        str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));
        for (std::size_t i = 1; i < str.size(); ++i) {
            str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
        }
    }
    jsonDoc.set(rootKey, str);
    return true;
}

bool String::toUpper(Data::JsonScope& jsonDoc) {
    auto str = jsonDoc.get<std::string>(rootKey).value_or("");
    std::ranges::transform(
        str,
        str.begin(),
        [](char const c) {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    );
    jsonDoc.set(rootKey, str);
    return true;
}

bool String::toLower(Data::JsonScope& jsonDoc) {
    auto str = jsonDoc.get<std::string>(rootKey).value_or("");
    std::ranges::transform(
        str,
        str.begin(),
        [](char const c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    );
    jsonDoc.set(rootKey, str);
    return true;
}

bool String::lPad(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return pad(args, jsonDoc, true);
}

bool String::rPad(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    return pad(args, jsonDoc, false);
}

bool String::lPadNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return pad(args, jsonDoc, true, true);
}

bool String::rPadNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return pad(args, jsonDoc, false, true);
}

bool String::lPadNonNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return pad(args, jsonDoc, true, false);
}

bool String::rPadNonNumeric(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    return pad(args, jsonDoc, false, false);
}

bool String::strip(Data::JsonScope& jsonDoc) {
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    std::string_view view = str;
    Utility::StringHandler::strip(view);
    jsonDoc.set(rootKey, view);
    return true;
}

bool String::lStrip(Data::JsonScope& jsonDoc) {
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    std::string_view view = str;
    Utility::StringHandler::lStrip(view);
    jsonDoc.set(rootKey, view);
    return true;
}

bool String::rStrip(Data::JsonScope& jsonDoc) {
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    std::string_view view = str;
    Utility::StringHandler::rStrip(view);
    jsonDoc.set(rootKey, view);
    return true;
}

bool String::substring(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    if (args.size() > 3){
        return false;
    }
    if (args.size() < 2) {
        return false;
    }

    auto const start = parseSize(args[1]);
    if (!start) {
        return false;
    }

    auto const count = args.size() == 3 ? parseSize(args[2]) : std::optional<std::size_t>{};
    if (args.size() == 3 && !count) {
        return false;
    }
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    if (start.value() >= str.size()) {
        jsonDoc.set(rootKey, "");
        return true;
    }
    if (count) {
        jsonDoc.set(rootKey, str.substr(start.value(), *count));
        return true;
    }
    auto const substr = str.substr(start.value());
    jsonDoc.set(rootKey, substr);
    return true;
}

bool String::replace(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto replacer = [&jsonDoc](std::string_view const target, std::string_view const replacement) {
        auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
        auto const replacedStr = Utility::StringHandler::replaceAll(str, target, replacement);
        jsonDoc.set(rootKey, replacedStr);
    };

    if (args.size() < 2 || args[1].empty()) return false;
    if (args.size() == 2){
        replacer(args[1], "");
    }
    else if (args.size() == 3) {
        replacer(args[1], args[2]);
    }
    else {
        // Args is larger than 3
        // Find arg equal to "->". All args before are target, all args after are replacement
        auto const it = std::ranges::find(args, std::string_view{"->"});
        if (it == args.end()) {
            return false; // invalid argument
        }
        auto const target = args.recombineSubspan(1, static_cast<std::size_t>(std::distance(args.begin(), it) - 1));
        auto const replacement = args.recombineSubspan(static_cast<std::size_t>(std::distance(args.begin(), it) + 1));
        if (target.empty() || replacement.empty()) {
            return false; // invalid argument
        }
        replacer(target, replacement);
    }
    return true;
}

bool String::strCountAppearance(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto const substring = args.size() > 1 ? args.recombineSubspan(1) : " ";
    auto str = jsonDoc.get<std::string>(rootKey).value_or("");
    std::size_t count = 0;
    while (!substring.empty() && !str.empty()) {
        auto const pos = str.find(substring);
        if (pos == std::string::npos) {
            break;
        }
        str.erase(pos, substring.length());
        count++;
    }
    jsonDoc.set(rootKey, count);
    return true;
}

bool String::split(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    if (args.size() > 2) {
        return false;
    }
    if (args.size() == 2 && args[1].size() != 1) {
        return false;
    }
    auto const delimiter = args.size() == 2 ? args[1].front() : ' ';
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.setEmptyArray(rootKey);
    for (auto [index, word] : Utility::StringHandler::split(str, delimiter) | Utility::Ranges::enumerate) {
        auto const indexedKey = rootKey.addIndex(index);
        jsonDoc.set(indexedKey, word);
    }
    return true;
}

bool String::splitNewlines(Data::JsonScope& jsonDoc) {
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.setEmptyArray(rootKey);
    auto const lines = Utility::StringHandler::split(str, '\n');
    for (auto [index, line] : lines | Utility::Ranges::enumerate) {
        bool const needsNormalizing = line.ends_with('\r') && index + 1 < lines.size();
        auto const normalized = needsNormalizing ? line.substr(0, line.size() - 1) : line;
        jsonDoc.set(rootKey.addIndex(index), normalized);
    }
    return true;
}

//------------------------------------------
// strcompare

bool String::strcompareMembers(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc){
    if (args.size() < 2) return false; // Trivial case: no keys provided
    bool const equal = args.subspan(1)
        | std::views::transform([&jsonDoc](std::string_view const key) -> std::optional<std::string> {
            if (auto expectedStr = jsonDoc.get<std::string>(rootKey.addMember(key)); expectedStr.has_value()) {
                return std::optional{expectedStr.value()};
            }
            return std::nullopt;
        })
        | Utility::Ranges::allEqualAnd([](auto const& opt) { return opt.has_value(); });
    jsonDoc.set(rootKey, equal);
    return true;
}

bool String::strcompareEquals(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto const compareStr = args.recombineSubspan(1);
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.set(rootKey, str == compareStr);
    return true;
}

bool String::strcompareContains(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto const compareStr = args.size() > 1 ? args.recombineSubspan(1) : " ";
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.set(rootKey, str.contains(compareStr));
    return true;
}

bool String::strcompareStartsWith(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto const compareStr = args.size() > 1 ? args.recombineSubspan(1) : " ";
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.set(rootKey, str.starts_with(compareStr));
    return true;
}

bool String::strcompareEndsWith(Utility::Args::SegmentedStringView const& args, Data::JsonScope& jsonDoc) {
    auto const compareStr = args.size() > 1 ? args.recombineSubspan(1) : " ";
    auto const str = jsonDoc.get<std::string>(rootKey).value_or("");
    jsonDoc.set(rootKey, str.ends_with(compareStr));
    return true;
}

} // namespace Nebulite::Module::Transformation
