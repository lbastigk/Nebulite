#include <cctype>
#include <algorithm>

#include "Data/Document/JsonScope.hpp"
#include "Module/Transformation/String.hpp"

namespace Nebulite::Module::Transformation {

void String::bindTransformations() {
    BIND_TRANSFORMATION_STATIC(&toUpper, toUpperName, toUpperDesc);
    BIND_TRANSFORMATION_STATIC(&toLower, toLowerName, toLowerDesc);
    BIND_TRANSFORMATION_STATIC(&lPad, lPadName, lPadDesc);
    BIND_TRANSFORMATION_STATIC(&rPad, rPadName, rPadDesc);
    BIND_TRANSFORMATION_STATIC(&lPadNumeric, lPadNumericName, lPadNumericDesc);
    BIND_TRANSFORMATION_STATIC(&rPadNumeric, rPadNumericName, rPadNumericDesc);
    BIND_TRANSFORMATION_STATIC(&lPadNonNumeric, lPadNonNumericName, lPadNonNumericDesc);
    BIND_TRANSFORMATION_STATIC(&rPadNonNumeric, rPadNonNumericName, rPadNonNumericDesc);
    BIND_TRANSFORMATION_STATIC(&strip, trimName, trimDesc);
    BIND_TRANSFORMATION_STATIC(&lStrip, lStripName, lStripDesc);
    BIND_TRANSFORMATION_STATIC(&rStrip, rStripName, rStripDesc);
    BIND_TRANSFORMATION_STATIC(&substring, substringName, substringDesc);
    BIND_TRANSFORMATION_STATIC(&replace, replaceName, replaceDesc);
    BIND_TRANSFORMATION_STATIC(&strCountAppearance, strCountAppearanceName, strCountAppearanceDesc);

    transformationFuncTree->bindCategory(strcompareName, strcompareDesc);
    BIND_TRANSFORMATION_STATIC(&strcompareEquals, strcompareEqualsName, strcompareEqualsDesc);
    BIND_TRANSFORMATION_STATIC(&strcompareContains, strcompareContainsName, strcompareContainsDesc);
    BIND_TRANSFORMATION_STATIC(&strcompareStartsWith, strcompareStartsWithName, strcompareStartsWithDesc);
    BIND_TRANSFORMATION_STATIC(&strcompareEndsWith, strcompareEndsWithName, strcompareEndsWithDesc);
}

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
bool String::lPad(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (args.size() < 2) {
        return false;
    }
    auto const size = std::stoul(args[1]);
    auto const padChar = args.size() >= 3 ? args[2][0] : ' ';
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (str.size() >= size) {
        return true; // No padding needed
    }
    str = std::string(size - str.size(), padChar) + str;
    jsonDoc->set(rootKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::rPad(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2) {
        return false;
    }
    auto const size = std::stoul(args[1]);
    auto const padChar = args.size() >= 3 ? args[2][0] : ' ';
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (str.size() >= size) {
        return true; // No padding needed
    }
    str = str + std::string(size - str.size(), padChar);
    jsonDoc->set(rootKey, str);
    return true;
}

bool String::lPadNumeric(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
        return lPad(args, jsonDoc);
    }
    return true; // Not numeric, but not an error either, so we return true without modifying the string
}

bool String::rPadNumeric(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
            return rPad(args, jsonDoc);
    }
    return true; // Not numeric, but not an error either, so we return true without modifying the string
}

bool String::lPadNonNumeric(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (!Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
        return lPad(args, jsonDoc);
    }
    return true; // numeric, but not an error either, so we return true without modifying the string
}

bool String::rPadNonNumeric(std::span<std::string const> const& args, Data::JsonScope* jsonDoc){
    if (!Utility::StringHandler::isNumber(jsonDoc->get<std::string>(rootKey).value_or(""))) {
        return rPad(args, jsonDoc);
    }
    return true; // numeric, but not an error either, so we return true without modifying the string
}

// NOLINTNEXTLINE
bool String::strip(Data::JsonScope* jsonDoc) {
    auto const str = Utility::StringHandler::strip(jsonDoc->get<std::string>(rootKey).value_or(""));
    jsonDoc->set(rootKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::lStrip(Data::JsonScope* jsonDoc) {
    auto const str = Utility::StringHandler::lStrip(jsonDoc->get<std::string>(rootKey).value_or(""));
    jsonDoc->set(rootKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::rStrip(Data::JsonScope* jsonDoc) {
    auto const str = Utility::StringHandler::rStrip(jsonDoc->get<std::string>(rootKey).value_or(""));
    jsonDoc->set(rootKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::substring(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3){
        return false;
    }
    auto const start = std::stoul(args[1]);
    size_t const length = std::stoul(args[2]);
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (start >= str.size()) {
        jsonDoc->set(rootKey, "");
        return true;
    }
    auto const substr = str.substr(start, length);
    jsonDoc->set(rootKey, substr);
    return true;
}

// NOLINTNEXTLINE
bool String::replace(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() != 3){
        return false;
    }
    auto const& target = args[1];
    auto const& replacement = args[2];
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    auto const replacedStr = Utility::StringHandler::replaceAll(str, target, replacement);
    jsonDoc->set(rootKey, replacedStr);
    return true;
}

bool String::strCountAppearance(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2){
        return false;
    }
    auto const substring = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto str = jsonDoc->get<std::string>(rootKey).value_or("");
    size_t count = 0;
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

bool String::strcompareEquals(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2){
        return false;
    }
    auto const compareStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str == compareStr);
    return true;
}

bool String::strcompareContains(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2){
        return false;
    }
    auto const compareStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str.find(compareStr) != std::string::npos);
    return true;
}

bool String::strcompareStartsWith(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2){
        return false;
    }
    auto const compareStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    jsonDoc->set(rootKey, str.rfind(compareStr, 0) == 0);
    return true;
}

bool String::strcompareEndsWith(std::span<std::string const> const& args, Data::JsonScope* jsonDoc) {
    if (args.size() < 2){
        return false;
    }
    auto const compareStr = Utility::StringHandler::recombineArgs(args.subspan(1));
    auto const str = jsonDoc->get<std::string>(rootKey).value_or("");
    if (compareStr.size() > str.size()) {
        jsonDoc->set(rootKey, false);
        return true;
    }
    jsonDoc->set(rootKey, str.compare(str.size() - compareStr.size(), compareStr.size(), compareStr) == 0);
    return true;
}

} // namespace Nebulite::Module::Transformation
