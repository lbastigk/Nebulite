#ifndef SCOPEACCESSOR_TPP
#define SCOPEACCESSOR_TPP

//------------------------------------------
// Includes

// Standard library
#include <utility>

// Nebulite
#include "Data/Document/ScopedKey.hpp"

//------------------------------------------
namespace Nebulite {

template <typename DomainType>
ScopeAccessor::DomainModuleToken<DomainType>::DomainModuleToken(Module::Base::DomainModule<DomainType> const& dm){
    // Root for a provided scope, type: DomainModule
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
#endif // SCOPEACCESSOR_TPP
