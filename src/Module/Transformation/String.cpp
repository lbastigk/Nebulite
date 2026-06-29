//------------------------------------------
// Includes

// Standard library
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <span>
#include <string>

// Nebulite
#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/String.hpp"
#include "Utility/StringHandler.hpp"

//------------------------------------------
namespace Nebulite::Module::Transformation {

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

    bindCategory(strcompareName, strcompareDesc);
    bindTransformation(&strcompareEquals, strcompareEqualsName, strcompareEqualsDesc);
    bindTransformation(&strcompareContains, strcompareContainsName, strcompareContainsDesc);
    bindTransformation(&strcompareStartsWith, strcompareStartsWithName, strcompareStartsWithDesc);
    bindTransformation(&strcompareEndsWith, strcompareEndsWithName, strcompareEndsWithDesc);
}

bool String::strLen(Data::JsonScope* jsonDoc){
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str.size());
    return true;
}

bool String::capitalize(Data::JsonScope* jsonDoc){
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (!str.empty()) {
        str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));
        for (std::size_t i = 1; i < str.size(); ++i) {
            str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
        }
    }
    jsonDoc->set(rootKey, str);
    return true;
}

bool String::toUpper(Data::JsonScope* jsonDoc) {
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    std::ranges::transform(
        str,
        str.begin(),
        [](char const c) {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    );
    jsonDoc->set(rootKey, str);
    return true;
}

bool String::toLower(Data::JsonScope* jsonDoc) {
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    std::ranges::transform(
        str,
        str.begin(),
        [](char const c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    );
    jsonDoc->set(rootKey, str);
    return true;
}

bool String::lPad(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (args.size() < 2) {
        return false;
    }
    auto const size = std::stoul(std::string(args[1]));
    auto const padChar = args.size() >= 3 ? args[2][0] : ' ';
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (str.size() >= size) {
        return true; // No padding needed
    }
    str = std::string(size - str.size(), padChar) + str;
    jsonDoc->set(rootKey, str);
    return true;
}

bool String::rPad(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    auto const size = std::stoul(std::string(args[1]));
    auto const padChar = args.size() >= 3 ? args[2][0] : ' ';
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (str.size() >= size) {
        return true; // No padding needed
    }
    str = str + std::string(size - str.size(), padChar);
    jsonDoc->set(rootKey, str);
    return true;
}

bool String::lPadNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
        return lPad(args, jsonDoc);
    }
    return true; // Not numeric, but not an error either, so we return true without modifying the string
}

bool String::rPadNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
            return rPad(args, jsonDoc);
    }
    return true; // Not numeric, but not an error either, so we return true without modifying the string
}

bool String::lPadNonNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (!Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
        return lPad(args, jsonDoc);
    }
    return true; // numeric, but not an error either, so we return true without modifying the string
}

bool String::rPadNonNumeric(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (!Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
        return rPad(args, jsonDoc);
    }
    return true; // numeric, but not an error either, so we return true without modifying the string
}

bool String::strip(Data::JsonScope* jsonDoc) {
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    std::string_view view = str;
    Utility::StringHandler::strip(view);
    jsonDoc->set(rootKey, view);
    return true;
}

bool String::lStrip(Data::JsonScope* jsonDoc) {
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    std::string_view view = str;
    Utility::StringHandler::lStrip(view);
    jsonDoc->set(rootKey, view);
    return true;
}

bool String::rStrip(Data::JsonScope* jsonDoc) {
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    std::string_view view = str;
    Utility::StringHandler::rStrip(view);
    jsonDoc->set(rootKey, view);
    return true;
}

bool String::substring(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() > 3){
        return false;
    }
    if (args.size() < 2) {
        return false;
    }

    auto const start = std::stoul(std::string(args[1]));

    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (start >= str.size()) {
        jsonDoc->set(rootKey, "");
        return true;
    }
    auto const substr = args.size() == 3 ? str.substr(start, std::stoul(std::string(args[2]))) : str.substr(start);
    jsonDoc->set(rootKey, substr);
    return true;
}

bool String::replace(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    auto replacer = [jsonDoc](std::string_view const target, std::string_view const replacement) {
        auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
        auto const replacedStr = Utility::StringHandler::replaceAll(str, target, replacement);
        jsonDoc->set(rootKey, replacedStr);
    };

    if (args.size() < 2) return false;
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
        auto const target = args.subspan(1, static_cast<std::size_t>(std::distance(args.begin(), it) - 1));
        auto const replacement = args.subspan(static_cast<std::size_t>(std::distance(args.begin(), it) + 1));
        if (target.empty() || replacement.empty()) {
            return false; // invalid argument
        }
        replacer(Utility::StringHandler::recombineArgs(target), Utility::StringHandler::recombineArgs(replacement));
    }
    return true;
}

bool String::strCountAppearance(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    auto const substring = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : " ";
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    std::size_t count = 0;
    while (!substring.empty() && !str.empty()) {
        auto const pos = str.find(substring);
        if (pos == std::string::npos) {
            break;
        }
        str.erase(pos, substring.length());
        count++;
    }
    jsonDoc->set(rootKey, count);
    return true;
}

bool String::split(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc){
    if (args.size() > 2) {
        return false;
    }
    if (args.size() == 2 && args[1].size() > 1) {
        return false;
    }
    auto const delimiter = args.size() == 2 ? args[1].front() : ' ';
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->setEmptyArray(rootKey);
    for (auto [index, word] : Utility::StringHandler::split(str, delimiter) | std::views::enumerate) {
        auto const indexedKey = rootKey.addIndex(static_cast<size_t>(index));
        jsonDoc->set(indexedKey, word);
    }
    return true;
}

//------------------------------------------
// strcompare

bool String::strcompareEquals(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    auto const compareStr = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : "";
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str == compareStr);
    return true;
}

bool String::strcompareContains(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    auto const compareStr = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : " ";
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str.contains(compareStr));
    return true;
}

bool String::strcompareStartsWith(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    auto const compareStr = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : " ";
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str.starts_with(compareStr));
    return true;
}

bool String::strcompareEndsWith(std::span<std::string_view const> const& args, Data::JsonScope* jsonDoc) {
    auto const compareStr = args.size() > 1 ? Utility::StringHandler::recombineArgs(args.subspan(1)) : " ";
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str.ends_with(compareStr));
    return true;
}

} // namespace Nebulite::Module::Transformation
