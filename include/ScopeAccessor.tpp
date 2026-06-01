#ifndef SCOPEACCESSOR_TPP
#define SCOPEACCESSOR_TPP

//------------------------------------------
// Includes

// Nebulite
#include "Data/Document/ScopedKey.hpp"
#include "Utility/CompileTimeEvaluate.hpp"

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
        static_assert(Utility::CompileTimeEvaluate::always_false(), "Unsupported DomainType for DomainModuleToken. Please add a specialization for it in the constructor.");
    }
}

} // namespace Nebulite
#endif // SCOPEACCESSOR_TPP
