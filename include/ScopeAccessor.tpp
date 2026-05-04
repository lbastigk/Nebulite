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
    static auto constexpr root = Data::ScopedKeyView("providedScope.module.domain");

    if constexpr (std::is_same_v<DomainType, Core::GlobalSpace>) {
        prefix = root.addMember("globalSpace").addMember(dm.moduleScope.getScopePrefix()).toString();
    } else if constexpr (std::is_same_v<DomainType, Core::RenderObject>) {
        prefix = root.addMember("renderObject").addMember(dm.moduleScope.getScopePrefix()).toString();
    } else {
        // Unsupported DomainType, please add the specialization for it in this constructor
        std::unreachable();
    }
}

} // namespace Nebulite
#endif // NEBULITE_SCOPE_ACCESSOR_TPP
