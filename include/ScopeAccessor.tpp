#ifndef NEBULITE_SCOPE_ACCESSOR_TPP
#define NEBULITE_SCOPE_ACCESSOR_TPP

//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite {

template <typename DomainType>
ScopeAccessor::DomainModuleToken<DomainType>::DomainModuleToken(Interaction::Execution::DomainModule<DomainType> const& dm){
    if constexpr (std::is_same_v<DomainType, Core::GlobalSpace>) {
        prefix = "" + dm.moduleScope.getScopePrefix();
    } else if constexpr (std::is_same_v<DomainType, Core::RenderObject>) {
        prefix = "providedScope.domainModule.renderObject."  + dm.moduleScope.getScopePrefix();
    } else {
        // Unsupported DomainType, please add the specialization for it in this constructor
        std::unreachable();
    }
}

} // namespace Nebulite
#endif // NEBULITE_SCOPE_ACCESSOR_TPP
