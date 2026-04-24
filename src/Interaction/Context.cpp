#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Interaction {

std::string_view ContextDeriver::stripContext(std::string_view const& str){
    auto [targetType, targetString] = getTypeAndPrefixFromString(str);
    // Don't strip context for resource variables or a none-context, as the context is needed for the link
    if (targetType == TargetType::resource || targetType == TargetType::none) {
        return str;
    }
    if (str.size() <= targetString.size()) {
        return "";
    }
    return str.substr(targetString.size() + 1);
}

ContextDeriver::TargetType ContextDeriver::getTypeFromString(std::string_view const& str){
    return getTypeAndPrefixFromString(str).first;
}

std::pair<ContextDeriver::TargetType, std::string_view> ContextDeriver::getTypeAndPrefixFromString(std::string_view const& str){
    auto const positionPipingOperator = str.find(Data::JSON::SpecialCharacter::transformationPipe);
    auto const positionContextKeySeparator = str.find(contextKeySeparator);

    // Find context string
    auto firstSeparatorPosition = std::min(positionPipingOperator, positionContextKeySeparator);
    if (firstSeparatorPosition == std::string_view::npos) {
        firstSeparatorPosition = str.size() + 1;
    }

    // Determine TargetType based on string
    if (firstSeparatorPosition == 0) {
        return {TargetType::none, str};
    }
    std::string_view const context = str.substr(0, firstSeparatorPosition);

    auto const it = std::ranges::find_if(contextPrefixPairs, [&](auto const p) {
        return context == p.second;
    });
    if (it != contextPrefixPairs.end()) {
        return *it;
    }
    return {TargetType::resource, str}; // All other prefixes are considered type resource
}

void ContextScope::combineAll(Data::JsonScope& merged) const {
    Data::ScopedKey const contextSelf("self.");
    Data::ScopedKey const contextOther("other.");
    Data::ScopedKey const contextGlobal("global.");
    merged.setSubDoc(contextSelf, self);
    merged.setSubDoc(contextOther, other);
    merged.setSubDoc(contextGlobal, global);
}

void ContextScope::combineLocal(Data::JsonScope& merged) const {
    Data::ScopedKey const contextSelf("self.");
    Data::ScopedKey const contextOther("other.");
    merged.setSubDoc(contextSelf, self);
    merged.setSubDoc(contextOther, other);
}

} // namespace Nebulite::Interaction
