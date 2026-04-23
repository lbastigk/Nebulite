#include "Data/Document/JsonScope.hpp"
#include "Interaction/Context.hpp"
#include "Interaction/Execution/Domain.hpp"

namespace Nebulite::Interaction {

ContextDeriver::Type ContextDeriver::getTypeFromString(std::string_view const& str){
    if (str.starts_with(startSelf)) {
        return Type::self;
    }
    if (str.starts_with(startOther)) {
        return Type::other;
    }
    if (str.starts_with(startGlobal)) {
        return Type::global;
    }
    return Type::resource; // Any other start is assumed to be type resource
}

std::pair<ContextDeriver::Type, std::string_view> ContextDeriver::getTypeAndPrefixFromString(std::string_view const& str){
    if (str.starts_with(startSelf)) {
        return {Type::self, startSelf};
    }
    if (str.starts_with(startOther)) {
        return {Type::other, startOther};
    }
    if (str.starts_with(startGlobal)) {
        return {Type::global, startGlobal};
    }
    return {Type::resource, ""}; // Any other start is assumed to be type resource
}

ContextScope Context::demote() const {
    return ContextScope{
        self.domainScope.shareScope(""),
        other.domainScope.shareScope(""),
        global.domainScope.shareScope("")
    };
}

} // namespace Nebulite::Interaction
