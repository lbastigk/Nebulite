#ifndef MODULE_BASE_DOMAINMODULE_TPP
#define MODULE_BASE_DOMAINMODULE_TPP

//------------------------------------------
// Includes

// Nebulite
#include "Module/Base/DomainModuleBase.hpp"

//------------------------------------------
// Conditional includes

#ifndef MODULE_BASE_DOMAINMODULE_HPP
    #include "Module/Base/DomainModule.hpp"
#endif // MODULE_BASE_DOMAINMODULE_HPP

//------------------------------------------
namespace Nebulite::Module::Base {

template <typename DomainType>
DomainModule<DomainType>::DomainModule(ConstructorParams const& params)
    : DomainModuleBase(
        params.funcTreePtr,
        params.scope,
        params.settings
    ),
    moduleName(std::move(params.name)),
    domain(params.domainReference) {}

template <typename DomainType>
DomainModule<DomainType>::~DomainModule() = default;

} // namespace Nebulite::Module::Base
#endif // MODULE_BASE_DOMAINMODULE_TPP
