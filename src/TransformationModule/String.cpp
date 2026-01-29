#include <cctype>
#include <algorithm>

#include "TransformationModule/String.hpp"
#include "Core/JsonScope.hpp"

namespace Nebulite::TransformationModule {

void String::bindTransformations() {}

// NOLINTNEXTLINE
bool String::toUpper(Core::JsonScope* jsonDoc) {
    auto str = jsonDoc->get<std::string>(valueKey);
    std::ranges::transform(
        str,
        str.begin(),
        [](char const c) {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    );
    jsonDoc->set(valueKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::toLower(Core::JsonScope* jsonDoc) {
    auto str = jsonDoc->get<std::string>(valueKey);
    std::ranges::transform(
        str,
        str.begin(),
        [](char const c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    );
    jsonDoc->set(valueKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::strip(Core::JsonScope* jsonDoc) {
    auto const str = Utility::StringHandler::strip(jsonDoc->get<std::string>(valueKey));
    jsonDoc->set(valueKey, str);
    return true;
}

// NOLINTNEXTLINE
bool String::substring(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 3){
        return false;
    }
    auto const start = std::stoul(args[1]);
    size_t const length = std::stoul(args[2]);
    auto const str = jsonDoc->get<std::string>(valueKey);
    if (start >= str.size()) {
        jsonDoc->set(valueKey, "");
        return true;
    }
    auto const substr = str.substr(start, length);
    jsonDoc->set(valueKey, substr);
    return true;
}

// NOLINTNEXTLINE
bool String::replace(std::span<std::string const> const& args, Core::JsonScope* jsonDoc) {
    if (args.size() != 3){
        return false;
    }
    auto const target = args[1];
    auto const replacement = args[2];
    auto const str = jsonDoc->get<std::string>(valueKey);
    auto const replacedStr = Utility::StringHandler::replaceAll(str, target, replacement);
    jsonDoc->set(valueKey, replacedStr);
    return true;
}

} // namespace Nebulite::TransformationModule
