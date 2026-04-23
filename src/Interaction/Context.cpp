#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Interaction {

std::string_view ContextDeriver::stripContext(std::string_view const& str){
    auto const it = std::ranges::find_if(contextPrefixPairs, [&](auto const p) {
            auto const& toCompare = p.second;
            return str.size() >= toCompare.size() && std::equal(toCompare.begin(), toCompare.end(), str.begin());
        });
    if (it != contextPrefixPairs.end()) {
        return str.substr(it->second.size());
    }
    return str;
}

ContextDeriver::TargetType ContextDeriver::getTypeFromString(std::string_view const& str){
    auto const it = std::ranges::find_if(contextPrefixPairs, [&](auto const p) {
        auto const& toCompare = p.second;
        return str.size() >= toCompare.size() && std::equal(toCompare.begin(), toCompare.end(), str.begin());
    });
    if (it != contextPrefixPairs.end()) {
        return it->first;
    }
    return TargetType::resource; // All other prefixes are considered type resource
}

std::pair<ContextDeriver::TargetType, std::string_view> ContextDeriver::getTypeAndPrefixFromString(std::string_view const& str){
    auto const it = std::ranges::find_if(contextPrefixPairs, [&](auto const p) {
        auto const& toCompare = p.second;
        return str.size() >= toCompare.size() && std::equal(toCompare.begin(), toCompare.end(), str.begin());
    });
    if (it != contextPrefixPairs.end()) {
        return {it->first, it->second};
    }
    return {TargetType::resource, ""}; // All other prefixes are considered type resource
}

ContextScope Context::demote() const {
    return ContextScope{
        self.domainScope.shareScope(""),
        other.domainScope.shareScope(""),
        global.domainScope.shareScope("")
    };
}

} // namespace Nebulite::Interaction
