#ifndef NEBULITE_SCOPE_ACCESSOR_TPP
#define NEBULITE_SCOPE_ACCESSOR_TPP

//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "ScopeAccessor.hpp"

//------------------------------------------
namespace Nebulite {

template <typename DomainType>
ScopeAccessor::DomainModuleToken<DomainType>::DomainModuleToken(Interaction::Execution::DomainModule<DomainType> const& dm){
    static std::string start = "providedScope.module.domain.";
    if constexpr (std::is_same_v<DomainType, Core::GlobalSpace>) {
        prefix = Data::ScopedKeyView::combineKeys(start + "globalSpace.", dm.moduleScope.getScopePrefix());
    } else if constexpr (std::is_same_v<DomainType, Core::RenderObject>) {
        prefix = Data::ScopedKeyView::combineKeys(start + "renderObject.", dm.moduleScope.getScopePrefix());
    } else {
        // Unsupported DomainType, please add the specialization for it in this constructor
        std::unreachable();
    }
}

} // namespace Nebulite
#endif // NEBULITE_SCOPE_ACCESSOR_TPP
