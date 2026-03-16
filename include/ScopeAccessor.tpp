#ifndef NEBULITE_SCOPE_ACCESSOR_TPP
#define NEBULITE_SCOPE_ACCESSOR_TPP

#include <utility>
#include "ScopeAccessor.hpp"

namespace Nebulite {

template <typename DomainType>
ScopeAccessor::DomainModuleToken<DomainType>::DomainModuleToken(Interaction::Execution::DomainModule<DomainType> const& dm){
    if constexpr (std::is_same_v<DomainType, Core::GlobalSpace>) {
        prefix = "" + dm.moduleScope.getScopePrefix();
    } else if constexpr (std::is_same_v<DomainType, Core::RenderObject>) {
        prefix = "providedScope.domainModule.renderObject."  + dm.moduleScope.getScopePrefix();
    } else if constexpr (std::is_same_v<DomainType, Data::JsonScope>) {
        prefix = "providedScope.domainModule.jsonScope." + dm.moduleScope.getScopePrefix();
    } else {
        // Unknown DomainType, please add the specialization for it in this constructor
        std::unreachable();
    }
}

} // namespace Nebulite
#endif // NEBULITE_SCOPE_ACCESSOR_TPP
