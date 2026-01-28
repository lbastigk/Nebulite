#ifndef NEBULITE_SCOPE_ACCESSOR_TPP
#define NEBULITE_SCOPE_ACCESSOR_TPP

#include <utility>
#include "ScopeAccessor.hpp"
#include "Constants/Asserts.hpp"

namespace Nebulite {

template <typename DomainType>
ScopeAccessor::DomainModuleToken<DomainType>::DomainModuleToken(Interaction::Execution::DomainModule<DomainType> const& dm){
    if constexpr (std::is_same_v<DomainType, Core::GlobalSpace>) {
        prefix = "" + dm.moduleScope.getScopePrefix();
    } else if constexpr (std::is_same_v<DomainType, Core::RenderObject>) {
        prefix = "providedScope.domainModule.renderObject."  + dm.moduleScope.getScopePrefix();
    } else if constexpr (std::is_same_v<DomainType, Core::JsonScope>) {
        prefix = "providedScope.domainModule.jsonScope." + dm.moduleScope.getScopePrefix();
    } else {
        static_assert(Constants::Assert::always_false<DomainType>, "ScopeAccessor::DomainModuleToken: Unsupported DomainType for DomainModuleToken");
        std::unreachable();
    }
}

} // namespace Nebulite
#endif // NEBULITE_SCOPE_ACCESSOR_TPP
